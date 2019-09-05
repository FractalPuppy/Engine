#include "GameObject.h"

#include "Application.h"
#include "ModuleProgram.h"
#include "ModuleEditor.h"
#include "ModuleCamera.h"
#include "ModuleTime.h"
#include "ModuleInput.h"
#include "ModuleScene.h"
#include "ModuleScript.h"
#include "ModuleTextures.h"
#include "ModuleResourceManager.h"
#include "ModuleRender.h"
#include "ModuleSpacePartitioning.h"
#include "ModuleAudioManager.h"
#include "ModuleNavigation.h"
#include "ModuleParticles.h"

#include "Component.h"
#include "ComponentTransform.h"
#include "ComponentCamera.h"
#include "ComponentLight.h"
#include "ComponentRenderer.h"
#include "ComponentTransform2D.h"
#include "ComponentText.h"
#include "ComponentImage.h"
#include "ComponentButton.h"
#include "ComponentAnimation.h"
#include "ComponentParticles.h"
#include "ComponentTrail.h"
#include "ComponentAudioListener.h"
#include "ComponentAudioSource.h"
#include "ComponentReverbZone.h"
#include "ComponentBoxTrigger.h"
#include "ComponentVolumetricLight.h"
#include "BaseScript.h"


#include "ResourceMesh.h"

#include "GUICreator.h"
#include "ResourceMaterial.h"
#include "ResourceMesh.h"
#include "ResourcePrefab.h"

#include "HashString.h"
#include "myQuadTree.h"
#include "AABBTree.h"
#include <stack>
#include "JSON.h"

#include "Math/float4x4.h"
#include "Math/MathFunc.h"
#include "Geometry/LineSegment.h"
#include "GL/glew.h"
#include "imgui.h"
#include "imgui_internal.h"

#define MAX_NAME 128
#define IMGUI_RIGHT_MOUSE_BUTTON 1

GameObject::GameObject(const char * name, unsigned uuid) : name(name), UUID(uuid)
{
}

GameObject::GameObject(const float4x4 & transform, const char * name, unsigned uuid) : name(name), UUID(uuid)
{
	this->transform = (ComponentTransform*)CreateComponent(ComponentType::Transform);
	this->transform->AddTransform(transform);
}

GameObject::GameObject(const GameObject & gameobject)
{
	name = gameobject.name;
	tag = gameobject.tag;
	UUID = App->scene->GetNewUID();
	parentUUID = gameobject.parentUUID;
	isStatic = gameobject.isStatic;
	activeInHierarchy = gameobject.activeInHierarchy;
	activeSelf = gameobject.activeSelf;
	isVolumetric = gameobject.isVolumetric;
	hasLight = gameobject.hasLight;
	isBoneRoot = gameobject.isBoneRoot;
	openInHierarchy = gameobject.openInHierarchy;
	animationIndexChannel = gameobject.animationIndexChannel;
	isPrefab = gameobject.isPrefab;
	isPrefabSync = gameobject.isPrefabSync;
	prefabUID = gameobject.prefabUID;

	if (isPrefab)
	{
		prefab = (ResourcePrefab*)App->resManager->Get(prefabUID);
		prefab->AddInstance(this);
	}

	assert(!(isVolumetric && hasLight));
	bbox = gameobject.bbox;
	navigable = gameobject.navigable;
	noWalkable = gameobject.noWalkable;

	for (const auto& component : gameobject.components)
	{
		Component *componentcopy = component->Clone();
		componentcopy->gameobject = this;
		components.push_back(componentcopy);
		switch (component->type)
		{
			case ComponentType::Transform:
				transform = (ComponentTransform*)componentcopy;
				break;

			case ComponentType::Button:
				{
					Button* button = (Button*)componentcopy;
					button->text->gameobject = this;
					button->buttonImage->gameobject = this;
					button->highlightedImage->gameobject = this;
					button->pressedImage->gameobject = this;
					button->rectTransform->gameobject = this;
				}
				break;

			case ComponentType::Light:
				light = (ComponentLight*)componentcopy;
				break;
			
			case ComponentType::VolumetricLight:
				((ComponentVolumetricLight*)componentcopy)->renderer->gameobject = this;
				break;
		}
	}

	if (GetComponentOld(ComponentType::Renderer) != nullptr || GetComponentOld(ComponentType::Light) != nullptr
		|| GetComponentOld(ComponentType::VolumetricLight) != nullptr)
	{
		App->scene->AddToSpacePartition(this);
	}

	for (const auto& child : gameobject.children)
	{
		GameObject* childcopy = new GameObject(*child);
		childcopy->parent = this;
		children.push_back(childcopy);
	}
}


GameObject::~GameObject()
{
	if (prefab != nullptr)
	{
		if (prefab->RemoveInstance(this))
		{
			App->resManager->DeleteResource(prefabUID);
		}
		prefab = nullptr;
	}

	for (auto &component : components)
	{
		RELEASE(component);
	}

	for (auto &child : children)
	{
		RELEASE(child);
	}

	components.clear();
	children.clear();

	transform = nullptr;
	parent = nullptr;
}

void GameObject::DrawProperties()
{
	char *go_name = new char[MAX_NAME];
	char *go_tag = new char[MAX_NAME];

	strcpy(go_name, name.c_str());
	strcpy(go_tag, tag.c_str());
	ImGui::InputText("Name", go_name, MAX_NAME);
	ImGui::InputText("TAG", go_tag, MAX_NAME);
	name = go_name;
	tag = go_tag;

	delete[] go_name;
	delete[] go_tag;

	if (this != App->scene->root || this != App->scene->canvas)
	{
		bool active = activeSelf;
		if (ImGui::Checkbox("Active", &active))
		{
			SetActive(active);
		}

		ImGui::SameLine();

		if (ImGui::Checkbox("isPrefab", &isPrefab))
		{
			if (isPrefab)
			{
				if (ParentPrefab())
				{
					isPrefab = false;
					App->editor->GenerateGenericPopUp("Error", "This GameObject is child of a prefab!");
				}
				else if (ChildPrefab())
				{
					isPrefab = false;
					App->editor->GenerateGenericPopUp("Error", "This GameObject is parent of a prefab!");
				}
				else
				{
					MarkAsPrefab();
				}
			}
			else
			{
				if (prefab != nullptr)
				{
					prefab->RemoveInstance(this);
					App->resManager->DeleteResource(prefabUID);
					prefab = nullptr;
				}
			}
		}

		if (isPrefab)
		{
			if (ImGui::Checkbox("Sync to Prefab", &isPrefabSync))
			{
				if (isPrefabSync && prefab != nullptr)
				{
					UpdateToPrefab(prefab->RetrievePrefab());
				}
			}
			if (App->time->gameState == GameState::RUN)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}
			if (ImGui::Button("Update Prefab"))
			{
				prefab->Update(this);
			}
			ImGui::SameLine();
			if (ImGui::Button("Set As New Prefab"))
			{
				prefab->RemoveInstance(this);
				App->resManager->DeleteResource(prefabUID);
				prefab = nullptr;
				prefabUID = 0;
				MarkAsPrefab();
			}
			if (App->time->gameState == GameState::RUN)
			{
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}

		if (isVolumetric && isStatic) 
		{
			ImGui::Checkbox("Navigable", &navigable);
			if (navigable)
			{
				//defines walls and this stuff
				ImGui::Checkbox("Is obstacle", &noWalkable);
			}
		}
		if (children.size() > 1)
		{
			ImGui::NewLine();
			ImGui::Checkbox("Show navigation options", &showNavOptions);
			if (showNavOptions)
			{
				if (ImGui::Button("Make childs static"))
				{
					SetStaticAllChildsWithMesh();
					App->spacePartitioning->kDTree.Calculate();
				}
				if (ImGui::Button("Make childs navigable"))
				{
					SetNavigableAllChildsWithMesh();
					App->spacePartitioning->kDTree.Calculate();
				}
				if (ImGui::Button("Make childs obstacles"))
				{
					SetObstacleAllChildsWithMesh();
					App->spacePartitioning->kDTree.Calculate();
				}
				if (ImGui::Button("Add childs to navMesh"))
				{
					AddAllNavigableChildsToNavMesh();
				}
			}
		}
		if (ImGui::Checkbox("Static", &isStatic))
		{
			if (isStatic && GetComponentOld(ComponentType::Renderer) != nullptr)
			{
				MakeObjectWithMeshStatic();
			}
			else if (!isStatic)
			{
				App->scene->dynamicGOs.insert(this);
				App->scene->staticGOs.erase(this);
				assert(!(hasLight && isVolumetric)); //Impossible combination
				if (hasLight)
				{
					App->spacePartitioning->aabbTreeLighting.InsertGO(this);
				}
				if (isVolumetric)
				{
					App->spacePartitioning->aabbTree.InsertGO(this);
				}
			}
			App->spacePartitioning->kDTree.Calculate();
		}
	}

	for (auto &component : components)
	{
		component->DrawProperties();
	}
}

void GameObject::PreUpdate()
{
	if (!isActive()) return;

	for (auto& component : components)
	{
		if (component->enabled && component->type != ComponentType::Script)
		{
			component->PreUpdate();
		}
	}

	for (const auto& child : children)
	{
		child->PreUpdate();
	}
}

void GameObject::Update()
{
	if (!isActive()) return;

	for (auto& component : components)
	{
		if (component->enabled && component->type != ComponentType::Script)
		{
			component->Update();
		}
	}

	for (const auto& child : children)
	{
		child->Update();
	}

	for (std::list<GameObject*>::iterator itChild = children.begin(); itChild != children.end();)
	{

		if ((*itChild)->copyFlag) //Copied GO
		{
			(*itChild)->copyFlag = false;
			GameObject *copy = new GameObject(**itChild);
			copy->parent = this;
			this->children.push_back(copy);

			std::vector<ComponentRenderer*> renderers;

			copy->LinkRendererToBones(renderers);

			for (const auto& renderer : renderers)
			{
				renderer->LinkBones();
			}

		}
		++itChild;
	}

}

void GameObject::SetActive(bool active)
{
	bool wasActive = isActive();
	activeSelf = active;
	OnChangeActiveState(wasActive);
}

void GameObject::OnChangeActiveState(bool wasActive)
{
	if (wasActive != isActive())
	{
		for (auto& child : children)
		{
			child->SetActiveInHierarchy(!wasActive);
		}
		for (auto& component : components)
		{
			if (!wasActive)
			{
				if (App->time->gameState == GameState::RUN && component->type == ComponentType::Script 
					&& !((Script*)component)->hasBeenAwoken)
				{
					Script* script = (Script*)component;
					script->Awake();
					script->hasBeenAwoken = true;
				}
				if (component->enabled)
				{
					component->OnEnable();
				}
			}
			else if (component->enabled)
			{
				component->OnDisable();
			}
		}
	}
}

void GameObject::SetActiveInHierarchy(bool active)
{
	bool wasActive = isActive();
	activeInHierarchy = active; 
	OnChangeActiveState(wasActive);
}

Component* GameObject::CreateComponent(ComponentType type, JSON_value* value, bool prefabTemplate)
{
	Component* component = nullptr;
	ComponentVolumetricLight* volLight = nullptr;
	switch (type)
	{
	case ComponentType::Transform:
		component = new ComponentTransform(this);
		this->transform = (ComponentTransform*)component;
		this->transform->UpdateTransform();
		break;
	case ComponentType::Renderer:
		if (!hasLight)
		{
			component = new ComponentRenderer(this);
		}
		else
		{
			LOG("Light + Renderer combination not allowed");
		}
		break;
	case ComponentType::Light:
		if (!hasLight && !isVolumetric)
		{
			component = new ComponentLight(this);
			if (!prefabTemplate)
			{
				App->scene->lights.push_back((ComponentLight*)component);
				App->spacePartitioning->aabbTreeLighting.InsertGO(this);
			}
			hasLight = true;
			light = (ComponentLight*)component;
			movedFlag = true;
		}
		else
		{
			if (isVolumetric)
			{
				LOG("Light + Renderer combination not allowed");
			}
			else
			{
				LOG("Only 1 light component allowed");
			}
		}
		break;
	case ComponentType::Camera:
		component = new ComponentCamera(this);
		if (App->scene->maincamera == nullptr && !prefabTemplate)
		{
			App->scene->maincamera = (ComponentCamera*)component;
			App->scene->maincamera->isMainCamera = true;
		}
		break;
	case ComponentType::Animation:
		component = new ComponentAnimation(this);
		isBoneRoot = true;
		break;
	case ComponentType::Transform2D:
		component = new Transform2D(this);
		break;
	case ComponentType::Text:
		component = new Text(this);
		break;
	case ComponentType::Image:
		component = new ComponentImage(this);
		break;
	case ComponentType::Button:
		component = new Button(this);
		break;
	case ComponentType::Script:
		{
			assert(value != nullptr); //Only used for loading from json
			std::string name = value->GetString("script");
			Script* script = App->scripting->GetScript(name);
			if (script)
			{
				script->SetGameObject(this);
				component = (Component*)script;
				if (prefabTemplate)
				{
					App->scripting->RemoveScript(script, name);
				}
				//if is prefab then remove from componentlist + remove script don't rest reference if not found
			}
			else
			{
				LOG("%s script not loaded", name.c_str());
			}
		}
		break;
	case ComponentType::Particles:
		component = new ComponentParticles(this);
		break;
	case ComponentType::Trail:
		component = new ComponentTrail(this);
		break;
	case ComponentType::AudioSource:
		component = new ComponentAudioSource(this);
		break;
	case ComponentType::AudioListener:
		component = new ComponentAudioListener(this);
		App->audioManager->audioListeners.push_back((ComponentAudioListener*)component);
		if (App->audioManager->audioListeners.size() == 1u)
		{
			App->audioManager->mainListener = (ComponentAudioListener*)component;
			App->audioManager->mainListener->isMainListener = true; 
		}
		break;
	case ComponentType::ReverbZone:
		component = new ComponentReverbZone(this);
		App->audioManager->reverbZones.push_back((ComponentReverbZone*)component);
		break;
	case ComponentType::BoxTrigger:
		component = new ComponentBoxTrigger(this);
		break;
	case ComponentType::VolumetricLight:
		component = new ComponentVolumetricLight(this);
		volLight = (ComponentVolumetricLight*)component;
		break;
	default:
		break;
	}
	if (component != nullptr)
	{
		components.push_back(component);
	}

	if (volLight) //Spawn default vol. light after the component creation is completed
		volLight->UpdateMesh();
	return component;
}

std::vector<Component *> GameObject::GetComponents(ComponentType type) const
{
	std::vector<Component *> list;
	for (auto &component : components)
	{
		if (component->type == type)
		{
			list.push_back(component);
		}
	}
	return list;
}

std::vector<Component*> GameObject::GetComponentsInChildren(ComponentType type) const
{
	std::vector<Component *> list;
	std::queue<const GameObject *>GOs;
	GOs.push(this);
	while (!GOs.empty())
	{
		const GameObject* go = GOs.front();
		GOs.pop();
		for (const auto &component : go->components)
		{
			if (component->type == type)
			{
				list.push_back(component);
			}
		}
		for (const auto &child : go->children)
		{
			GOs.push(child);
		}
	}
	return list;
}

void GameObject::RemoveComponent(const Component& component)
{
	Component* trash = nullptr;
	std::vector<Component*>::iterator trashIt;
	for (std::vector<Component*>::iterator it = components.begin(); it != components.end(); ++it)
	{
		if (*it == &component)
		{
			(*it)->CleanUp();
			trash = *it; // Delete elements of an iterated container causes crashes inside the loop
			trashIt = it;
			break;
		}
	}
	if (trash != nullptr) // Safely remove component
	{
		if (trash->type == ComponentType::Light)
		{
			App->spacePartitioning->aabbTreeLighting.ReleaseNode(treeNode);
			light = nullptr;
			hasLight = false;
			treeNode = nullptr;
		}
		else if (trash->type == ComponentType::Renderer && treeNode != nullptr)
		{
			App->spacePartitioning->aabbTree.ReleaseNode(treeNode);
			treeNode = nullptr;
			isVolumetric = false;
		}
		components.erase(trashIt);
		RELEASE(trash);
	}
}

void GameObject::RemoveChild(GameObject* bastard)
{
	children.remove(bastard);
	RELEASE(bastard);
}

void GameObject::InsertChild(GameObject* child)
{
	children.push_back(child);
	child->parent = this;
}

void GameObject::ChangeParent(GameObject* oldParent, GameObject* newParent)
{
	oldParent->children.remove(this);
	newParent->children.push_back(this);
	this->parent = newParent;
	this->parentUUID = newParent->UUID;
	transform->NewAttachment();
	UpdateBBox();
}

ENGINE_API Component * GameObject::GetComponentOld(ComponentType type) const //Deprecated
{
	for (auto &component : components)
	{
		if (component->type == type)
		{
			return component;
		}
	}
	return nullptr;
}

ENGINE_API Component * GameObject::GetComponentInChildren(ComponentType type) const
{
	std::queue<const GameObject *>GOs;
	GOs.push(this);
	while (!GOs.empty())
	{
		const GameObject* go = GOs.front();
		GOs.pop();

		Component* component = go->GetComponentOld(type);
		if (component != nullptr) return component;

		for (const auto &child : go->children)
		{
			GOs.push(child);
		}
	}
	return nullptr;
}

void GameObject::UpdateGlobalTransform() //Updates global transform when moving
{
	float4x4 mytransform = GetLocalTransform();
	if (parent != nullptr)
	{
		mytransform = parent->GetGlobalTransform() * mytransform;
		if (transform != nullptr)
		{
			transform->global = mytransform;
		}
		UpdateBBox();
	}


	for (auto &child : children)
	{
		child->UpdateGlobalTransform();
	}
}

void GameObject::SetGlobalTransform(const float4x4 & global) //Replaces global transform
{
	movedFlag = true;
	if (transform != nullptr)
	{
		float4x4 parentglobal = float4x4::identity;
		if (parent != nullptr && parent->transform != nullptr)
		{
			parentglobal = parent->transform->global;
		}
		transform->SetGlobalTransform(global, parentglobal);

		for (GameObject* go : App->scene->selection)
		{
			if (go != App->scene->selected)
			{
				go->UpdateGlobalTransform();
			}
		}

		for (GameObject* go : children)
		{
			go->movedFlag = true;
		}
	}

}

float4x4 GameObject::GetGlobalTransform() const
{
	if (transform != nullptr)
		return transform->global;
	return float4x4::identity;
}

float4x4 GameObject::GetLocalTransform() const
{
	if (transform != nullptr)
		return transform->local;
	return float4x4::identity;
}

void GameObject::UpdateModel(unsigned int shader) const
{
	glUniformMatrix4fv(glGetUniformLocation(shader,
		"model"), 1, GL_TRUE, &GetGlobalTransform()[0][0]);
}

void GameObject::SetLightUniforms(unsigned shader) const
{
	std::unordered_set<GameObject*> lights;
	App->spacePartitioning->aabbTreeLighting.GetIntersections(bbox, lights);
	if (App->renderer->directionalLight)
	{
		lights.insert(App->renderer->directionalLight->gameobject);
	}

	unsigned directionals = 0u;
	unsigned points = 0u;
	unsigned spots = 0u;
	char buffer[32];
	//LOG("%s got %d lights", name.c_str(), lights.size());
	for (GameObject* go : lights)
	{
		assert(go->light != nullptr);

		if (!go->light->enabled) continue;
		switch (go->light->lightType)
		{
		case LightType::DIRECTIONAL:
			if (directionals == 0u)
			{
				memset(buffer, 0, 32);
				sprintf(buffer, "lights.directional[%d].direction", directionals);
				glUniform3fv(glGetUniformLocation(shader,
					buffer), 1, (GLfloat*)&go->light->direction);

				memset(buffer, 0, 32);
				sprintf(buffer, "lights.directional[%d].color", directionals);
				glUniform3fv(glGetUniformLocation(shader,
					buffer), 1, (GLfloat*)&go->light->color);

				memset(buffer, 0, 32);
				sprintf(buffer, "lights.directional[%d].intensity", points);
				glUniform1f(glGetUniformLocation(shader,
					buffer), go->light->intensity);
				++directionals;
			}
			break;
		case LightType::POINT:
			memset(buffer, 0, 32);
			sprintf(buffer, "lights.points[%d].position", points);
			glUniform3fv(glGetUniformLocation(shader,
				buffer), 1, (GLfloat*)&go->light->position);

			memset(buffer, 0, 32);
			sprintf(buffer, "lights.points[%d].direction", points);
			glUniform3fv(glGetUniformLocation(shader,
				buffer), 1, (GLfloat*)&go->light->direction);

			memset(buffer, 0, 32);
			sprintf(buffer, "lights.points[%d].color", points);
			glUniform3fv(glGetUniformLocation(shader,
				buffer), 1, (GLfloat*)&go->light->color);

			memset(buffer, 0, 32);
			sprintf(buffer, "lights.points[%d].radius", points);
			glUniform1f(glGetUniformLocation(shader,
				buffer), go->light->pointSphere.r);

			memset(buffer, 0, 32);
			sprintf(buffer, "lights.points[%d].intensity", points);
			glUniform1f(glGetUniformLocation(shader,
				buffer), go->light->intensity);

			++points;
			break;
		case LightType::SPOT:
			memset(buffer, 0, 32);
			sprintf(buffer, "lights.spots[%d].position", spots);
			glUniform3fv(glGetUniformLocation(shader,
				buffer), 1, (GLfloat*)&go->light->position);

			memset(buffer, 0, 32);
			sprintf(buffer, "lights.spots[%d].direction", spots);
			glUniform3fv(glGetUniformLocation(shader,
				buffer), 1, (GLfloat*)&go->light->direction);

			memset(buffer, 0, 32);
			sprintf(buffer, "lights.spots[%d].color", spots);
			glUniform3fv(glGetUniformLocation(shader,
				buffer), 1, (GLfloat*)&go->light->color);

			memset(buffer, 0, 32);
			sprintf(buffer, "lights.spots[%d].radius", spots);
			glUniform1f(glGetUniformLocation(shader,
				buffer), go->light->pointSphere.r);

			memset(buffer, 0, 32);
			sprintf(buffer, "lights.spots[%d].intensity", spots);
			glUniform1f(glGetUniformLocation(shader,
				buffer), go->light->intensity);

			memset(buffer, 0, 32);
			float innerRad = cosf(math::DegToRad(go->light->inner));
			sprintf(buffer, "lights.spots[%d].inner", spots);
			glUniform1fv(glGetUniformLocation(shader,
				buffer), 1, (GLfloat*)&innerRad);

			memset(buffer, 0, 32);
			float outerRad = cosf(math::DegToRad(go->light->outer));
			sprintf(buffer, "lights.spots[%d].outer", spots);
			glUniform1fv(glGetUniformLocation(shader,
				buffer), 1, (GLfloat*)&outerRad);

			++spots;
			break;
		}
	}
	glUniform1i(glGetUniformLocation(shader,
		"lights.num_directionals"), directionals);
	glUniform1i(glGetUniformLocation(shader,
		"lights.num_points"), points);
	glUniform1i(glGetUniformLocation(shader,
		"lights.num_spots"), spots);

	if (App->renderer->directionalLight && App->renderer->directionalLight->produceShadows)
	{
		glUniformMatrix4fv(glGetUniformLocation(shader,
			"lightProjView"), 1, GL_TRUE, &App->renderer->shadowsFrustum.ViewProjMatrix()[0][0]);

		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, App->renderer->shadowsTex);
		glUniform1i(glGetUniformLocation(shader, "shadowTex"), 6);
	}
}

void GameObject::LinkRendererToBones(std::vector<ComponentRenderer*>& renderers)
{
	ComponentRenderer* rend = GetComponent<ComponentRenderer>();

	if (rend != nullptr)
	{
		renderers.push_back(rend);
	}

	for (const auto& child : children)
	{
		child->LinkRendererToBones(renderers);
	}
}

void GameObject::LinkBones() const
{
	for (Component* c : GetComponentsInChildren(ComponentType::Renderer))
	{
		ComponentRenderer* cr = (ComponentRenderer*)c;
		if (cr->mesh != nullptr)
		{
			cr->LinkBones();
		}
	}
}

void GameObject::MarkAsPrefab()
{
	bool wasPrefab = false;

	if (prefabUID == 0)
	{
		prefabUID = App->scene->CreatePrefab(this);
	}
	else
	{
		wasPrefab = true;
	}

	prefab = (ResourcePrefab*)App->resManager->Get(prefabUID);
	if (prefab == nullptr)
	{
		prefabUID = App->scene->CreatePrefab(this);
		prefab = (ResourcePrefab*)App->resManager->Get(prefabUID);
		wasPrefab = false;
	}

	prefab->AddInstance(this);
	isPrefabSync = true;

	if (wasPrefab)
	{
		UpdateToPrefab(prefab->RetrievePrefab());
	}
}

bool GameObject::ChildPrefab() const
{
	if (this == App->scene->root) return false;
	for (const auto& child : children)
	{
		if (child->isPrefab)
		{
			return true;
		}
		else if(child->ChildPrefab())
		{
			return true;
		}
	}
	return false;
}

bool GameObject::ParentPrefab() const
{
	if (parent == App->scene->root || parent == nullptr) return false;
	if (parent->isPrefab)
	{
		return true;
	}
	return parent->ParentPrefab();
}

void GameObject::UpdateToPrefab(GameObject* prefabGo)
{
	if (prefabGo == nullptr)
	{
		LOG("Cannot update to prefab");
		return;
	}

	Component* myTransform = nullptr;
	if (transform != nullptr)
	{
		myTransform = transform->Clone();
	}
	else
	{
		Transform2D* transformUI = GetComponent<Transform2D>();
		if (transformUI != nullptr)
		{
			myTransform = transformUI->Clone();
		}
	}

	GameObject* myParent = parent;
	CleanUp();
	for (auto& component: components)
	{
		RELEASE(component);
	}
	for (auto& child : children)
	{
		RELEASE(child);
	}
	components.clear();
	children.clear();
	parent = myParent;

	if (myTransform != nullptr)
	{
		components.push_back(myTransform);
		if (myTransform->type == ComponentType::Transform)
		{
			transform = (ComponentTransform*)myTransform;
		}
	}

	for (const auto &component : prefabGo->components)
	{
		if (component->type == ComponentType::Transform ||
			component->type == ComponentType::Transform2D) continue;
		Component* newComponent = component->Clone();
		newComponent->gameobject = this;
		components.push_back(newComponent);
	}
	for (auto& child : prefabGo->children)
	{
		GameObject* newChild = new GameObject(*child);
		children.push_back(newChild);
		newChild->parent = this;
	}
	if (GetComponentOld(ComponentType::Renderer) != nullptr || GetComponentOld(ComponentType::Light) != nullptr)
	{
		App->scene->AddToSpacePartition(this);
	}

	LinkBones();
}

AABB GameObject::GetBoundingBox() const
{
	return bbox;
}

bool GameObject::Intersects(const LineSegment & line, float &distance, math::float3* intersectionPoint) const
{
	LineSegment localLine(line);
	localLine.Transform(GetGlobalTransform().Inverted());
	ComponentRenderer* mesh_renderer = (ComponentRenderer*)GetComponentOld(ComponentType::Renderer);
	if (mesh_renderer != nullptr)
	{
		if (mesh_renderer->mesh->Intersects(localLine, &distance, intersectionPoint))
		{
			if (intersectionPoint != nullptr)
			{
				math::float3 worldPoint = GetGlobalTransform().MulPos(*intersectionPoint);
				*intersectionPoint = worldPoint;
			}
			return true;
		}
	}
	return false;
}

bool GameObject::BboxIntersects(const GameObject* target) const
{
	bool intersects = false;
	bool allchecked = false;
	std::list<GameObject*>::const_iterator iti = children.begin();
	while (!intersects && !allchecked)
	{
		for (std::list<GameObject*>::const_iterator itj = target->children.begin(); itj != target->children.end(); ++itj)
		{
			if ((*iti)->bbox.Intersects((*itj)->bbox))
			{
				intersects = true;
				break;
			}
		}
		++iti;
		if (iti == children.end()) allchecked = true;
	}
	return intersects;
}

void GameObject::UpdateBBox()
{
	ComponentRenderer* renderer = (ComponentRenderer*)GetComponentOld(ComponentType::Renderer);
	if (renderer != nullptr)
	{
		if (renderer->mesh != nullptr)
			bbox = renderer->mesh->GetBoundingBox();

		bbox.TransformAsAABB(GetGlobalTransform());
	}
	ComponentVolumetricLight* volLight = (ComponentVolumetricLight*)GetComponentOld(ComponentType::VolumetricLight);
	if (volLight != nullptr)
	{
		if (volLight->renderer->mesh != nullptr)
			bbox = volLight->renderer->mesh->GetBoundingBox();

		bbox.TransformAsAABB(GetGlobalTransform());
	}
}

void GameObject::DrawBBox() const
{
	for (const auto& child : children)
	{
		child->DrawBBox();
	}

	ComponentRenderer *renderer = (ComponentRenderer*)GetComponentOld(ComponentType::Renderer);
	if (renderer != nullptr && renderer->mesh != nullptr)
	{
		if (renderer->mesh->GetReferences() > 0u)
			renderer->mesh->DrawBbox(App->program->defaultShader->id[0], bbox);
	}

	ComponentVolumetricLight* volLight = (ComponentVolumetricLight*)GetComponentOld(ComponentType::VolumetricLight);
	if (volLight != nullptr && volLight->renderer && volLight->renderer->mesh)
	{
		volLight->renderer->mesh->DrawBbox(App->program->defaultShader->id[0], bbox);
	}
}

bool GameObject::CleanUp()
{
	App->scene->DeleteFromSpacePartition(this);

	for (auto &component : components)
	{
		component->CleanUp();
	}

	for (auto &child : children)
	{
		child->CleanUp();
	}

	transform = nullptr;
	parent = nullptr;

	return true;
}

void GameObject::Save(JSON_value *gameobjects, bool selected) const
{
	if (parent != nullptr && App->scene->canvas != this && (App->scene->selected == this || !selected)) // we don't add gameobjects without parent (ex: World)
	{		
		JSON_value *gameobject = gameobjects->CreateValue();
		gameobject->AddUint("UID", UUID);
		gameobject->AddUint("ParentUID", parent->UUID);
		gameobject->AddString("Name", name.c_str());
		gameobject->AddString("Tag", tag.c_str());
		gameobject->AddUint("Static", isStatic);
		gameobject->AddUint("ActiveInHierarchy", activeInHierarchy);
		gameobject->AddUint("ActiveSelf", activeSelf);
		gameobject->AddUint("isBoneRoot", isBoneRoot);
		gameobject->AddFloat4x4("baseState", baseState);
		gameobject->AddUint("Navigable", navigable);
		gameobject->AddUint("No Walkable", noWalkable);
		gameobject->AddUint("openInHierarchy", openInHierarchy);

		gameobject->AddUint("isPrefab", isPrefab);
		gameobject->AddUint("isPrefabSync", isPrefabSync);
		gameobject->AddUint("prefabUID", prefabUID);

		JSON_value *componentsJSON = gameobject->CreateValue(rapidjson::kArrayType);
		for (auto &component : components)
		{
			JSON_value *componentJSON = componentsJSON->CreateValue();
			component->Save(componentJSON);
			componentsJSON->AddValue("", *componentJSON);
		}

		gameobject->AddValue("Components", *componentsJSON);
		gameobjects->AddValue("", *gameobject);
	}

	for (auto &child : children)
	{
		bool wasSelected = App->scene->selected == this;
		if (selected && App->scene->selected == this)
		{
			App->scene->selected = child;
		}
		child->Save(gameobjects, selected);
		if (wasSelected)
		{
			App->scene->selected = const_cast<GameObject*>(this);
		}
	}
}

void GameObject::Load(JSON_value *value, bool prefabTemplate)
{
	UUID = value->GetUint("UID");
	parentUUID = value->GetUint("ParentUID");
	name = value->GetString("Name");
	tag = value->GetString("Tag", tag.c_str());
	isStatic = value->GetUint("Static");
	activeInHierarchy = value->GetUint("ActiveInHierarchy", 1);
	activeSelf = value->GetUint("ActiveSelf", 1);
	isBoneRoot = value->GetUint("isBoneRoot");
	baseState = value->GetFloat4x4("baseState");
	navigable = value->GetUint("Navigable");
	noWalkable = value->GetUint("No Walkable");
	openInHierarchy = value->GetUint("openInHierarchy");

	isPrefab = value->GetUint("isPrefab");
	isPrefabSync = value->GetUint("isPrefabSync");
	prefabUID = value->GetUint("prefabUID");
	if (isPrefab && !prefabTemplate)
	{
		prefab = (ResourcePrefab*) App->resManager->Get(prefabUID);
		if (prefab != nullptr)
		{
			prefab->AddInstance(this);
		}
		else
		{
			LOG("Cannot find prefab with UID:%u", prefabUID);
			isPrefab = false;
		}
	}

	JSON_value* componentsJSON = value->GetValue("Components");
	ComponentVolumetricLight* volLight = nullptr;

	for (unsigned i = 0; i < componentsJSON->Size(); i++)
	{
		JSON_value* componentJSON = componentsJSON->GetValue(i);
		ComponentType type = (ComponentType)componentJSON->GetUint("Type");
		Component* component = CreateComponent(type, componentJSON, prefabTemplate);
		if (component)
		{
			component->Load(componentJSON);
			switch (type)
			{
			case ComponentType::VolumetricLight:
				volLight = (ComponentVolumetricLight*)component;
				break;
			case ComponentType::Particles:
				if (prefabTemplate)
				{
					App->particles->RemoveParticleSystem((ComponentParticles*)component);
				}
				break;
			case ComponentType::Trail:
				if (prefabTemplate)
				{
					App->particles->RemoveTrailRenderer((ComponentTrail*)component);
				}
				break;
			}
		}
	}

	if (volLight)
		volLight->UpdateMesh();

	if (isBoneRoot)
	{
		movedFlag = true;
	}

}

bool GameObject::IsParented(const GameObject & gameobject) const
{
	if (this == &gameobject)
	{
		return true;
	}
	for (const auto child : children)
	{
		if (child->IsParented(gameobject))
		{
			return true;
		}
	}
	return false;
}

void GameObject::DrawHierarchy()
{
	//if (parent != nullptr && parent->parent !=nullptr && parent->parent->isBoneRoot) return; //Direct bone access needed to put fx

	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | 
		(openInHierarchy ? ImGuiTreeNodeFlags_DefaultOpen: 0)
		| ImGuiTreeNodeFlags_OpenOnDoubleClick | (isSelected ? ImGuiTreeNodeFlags_Selected : 0);

	ImGui::PushID(this);
	if (!isActive())
	{
		if (isPrefab)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.4f, 0.4f, 1.00f));
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.00f));
		}
	}
	else if(isPrefab)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.8f, 0.8f, 1.00f));
	}

	if (children.empty())
	{
		node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	}
	ImGui::Text("|"); ImGui::SameLine();
	App->scene->DragNDropMove(this);
	openInHierarchy = ImGui::TreeNodeEx(this, node_flags, name.c_str());

	if (isSelected && ImGui::IsItemHovered() && App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_DOWN)
	{
		ImGui::OpenPopup("gameobject_options_popup");
	}
	if (ImGui::BeginPopup("gameobject_options_popup"))
	{
		GUICreator::CreateElements(this);
		if (ImGui::Selectable("Duplicate"))
		{
			App->scene->TakePhoto();
			for each (GameObject* go in App->scene->selection)
			{
				go->copyFlag = true;
			}
		}
		if (ImGui::Selectable("Delete"))
		{
			App->scene->TakePhoto();
			for each (GameObject* go in App->scene->selection)
			{
				go->deleteFlag = true;
			}
			App->scene->selected = nullptr;
			App->scene->selection.clear();
		}
		ImGui::EndPopup();
	}
	else if (ImGui::IsItemClicked() && (std::find(App->scene->selection.begin(), App->scene->selection.end(), this) == App->scene->selection.end() || App->input->IsKeyPressed(SDLK_LCTRL)))
	{
		App->scene->Select(this);
	}
	else if (!App->input->IsKeyPressed(SDLK_LCTRL))
	{
		App->scene->DragNDrop(this);
	}

	if (openInHierarchy)
	{
		for (auto &child : children)
		{
			child->DrawHierarchy();
		}
		if (!(node_flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
		{
			ImGui::TreePop();
		}
	}
	if (!isActive() || isPrefab)
	{
		ImGui::PopStyleColor();
	}
	ImGui::PopID();
}

void GameObject::SetStaticAncestors()
{
	std::stack<GameObject*> parents;
	parents.push(parent);
	while (!parents.empty() && parents.top() != App->scene->root)
	{
		GameObject* go = parents.top();
		go->isStatic = true;

		if (go->GetComponentOld(ComponentType::Renderer) != nullptr)
		{
			if (go->treeNode != nullptr && isVolumetric)
				App->spacePartitioning->aabbTree.ReleaseNode(go->treeNode);
			if (go->treeNode != nullptr && hasLight)
				App->spacePartitioning->aabbTreeLighting.ReleaseNode(go->treeNode);

		}
		parents.pop();
		parents.push(go->parent);
	}

}

void GameObject::OnPlay()
{
	//Go through all components letting them know play button has been pressed
	for (std::vector<Component*>::iterator it = components.begin(); it != components.end(); ++it)
	{
		(*it)->OnPlay();
	}

	//Recursive, this will only be executed on play
	for (std::list<GameObject*>::iterator it = children.begin(); it != children.end(); ++it)
	{
		(*it)->OnPlay();
	}
}

void GameObject::SetAllMoveFlags()
{
	for (const auto& child : children)
	{
		child->movedFlag = true;
		child->SetAllMoveFlags();
	}
}

void GameObject::UpdateTransforms(math::float4x4 parentGlobal)
{
	PROFILE;
	if (movedFlag)
	{
		if (App->time->gameState == GameState::STOP)
		{
			particlesDirty = true;
		}
		for (const auto& child : children)
		{
			if (!child->isStatic)
			{
				child->movedFlag = true;
			}
		}
		movedFlag = false;
		if (transform)
		{
			transform->local = math::float4x4::FromTRS(transform->position, transform->rotation, transform->scale);

			math::float4x4 global = math::float4x4::identity;
			if (this != App->scene->root && transform != nullptr)
			{
				math::float4x4 original = transform->global;
				transform->global = parentGlobal * transform->local;
				global = transform->global;
				transform->UpdateTransform();
				if (this == App->scene->selected)
				{
					transform->MultiSelectionTransform(global - original);
				}
			}

			UpdateBBox();

			if (!isStatic)
			{
				if (treeNode != nullptr && hasLight)
				{
					light->CalculateGuizmos();
					if (!treeNode->aabb.Contains(bbox))
					{
						App->spacePartitioning->aabbTreeLighting.ReleaseNode(treeNode);
						App->spacePartitioning->aabbTreeLighting.InsertGO(this);
					}
				}
				if (treeNode != nullptr && isVolumetric)
				{
					if (!treeNode->aabb.Contains(bbox))
					{
						App->spacePartitioning->aabbTree.ReleaseNode(treeNode);
						App->spacePartitioning->aabbTree.InsertGO(this);
					}
				}
			}
			else
			{
				App->spacePartitioning->kDTree.Calculate();
			}
		}
	}

	for (const auto& child : children)
	{
		if (transform != nullptr)
		{
			child->UpdateTransforms(transform->global);
		}
		else
		{
			child->UpdateTransforms(math::float4x4::identity);
		}
	}
}

bool GameObject::CheckDelete()
{
	//PROFILE;
	if (deleteFlag) //Delete GO
	{
		CleanUp();
		delete this;
		return true;
	}
	else
	{
		for (std::list<GameObject*>::iterator itChild = children.begin(); itChild != children.end();)
		{
			if ((*itChild)->CheckDelete())
			{
				children.erase(itChild++);
			}
			else
			{
				++itChild;
			}
		}
	}
	return false;
}


void GameObject::SetStaticAllChildsWithMesh()
{
	for (const auto& child : children)
	{
		if (child->isVolumetric)
		{
			if (!child->isStatic)
			{
				child->MakeObjectWithMeshStatic();
				child->isStatic = true;
			}
		}
		child->SetStaticAllChildsWithMesh();
	}
}
void GameObject::SetNavigableAllChildsWithMesh()
{
	for (const auto& child : children)
	{
		if (child->isVolumetric)
		{
			if (!child->isStatic)
			{
				child->MakeObjectWithMeshStatic();
				child->isStatic = true;
			}
			child->navigable = true;
			
		}
		child->SetNavigableAllChildsWithMesh();
	}
}
void GameObject::SetObstacleAllChildsWithMesh()
{
	for (const auto& child : children)
	{
		if (child->isVolumetric)
		{
			if (!child->isStatic)
			{
				child->MakeObjectWithMeshStatic();
				child->isStatic = true;
				
			}
			child->navigable = true;
			child->noWalkable = true;
			
		}
		child->SetObstacleAllChildsWithMesh();
	}
}

void GameObject::AddAllNavigableChildsToNavMesh()
{
	for (const auto& child : children)
	{
		if (child->isVolumetric && child->isStatic &&child->navigable)
		{
			App->navigation->AddNavigableMeshFromObject(child);
		}
		child->AddAllNavigableChildsToNavMesh();
	}
}

void GameObject::MakeObjectWithMeshStatic()
{
	SetStaticAncestors();
	App->scene->DeleteFromSpacePartition(this);
	App->scene->AddToSpacePartition(this);
	assert(!(hasLight && isVolumetric)); //Impossible combination
	if (hasLight && treeNode != nullptr)
	{
		App->spacePartitioning->aabbTreeLighting.ReleaseNode(treeNode);
	}
	if (isVolumetric && treeNode != nullptr)
	{
		App->spacePartitioning->aabbTree.ReleaseNode(treeNode);
	}
	LOG(name.c_str());
	std::string s = std::to_string(App->scene->staticGOs.size());
	LOG(s.c_str());
}