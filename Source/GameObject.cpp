#include "GameObject.h"

#include "Application.h"
#include "ModuleProgram.h"
#include "ModuleEditor.h"
#include "ModuleCamera.h"
#include "ModuleTime.h"
#include "ModuleInput.h"
#include "ModuleScene.h"
#include "ModuleTextures.h"
#include "ModuleRender.h"
#include "ModuleSpacePartitioning.h"
#include "ModuleAudioManager.h"
#include "ModuleNavigation.h"

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
#include "ComponentScript.h"
#include "ComponentParticles.h"
#include "ComponentTrail.h"
#include "ComponentAudioListener.h"
#include "ComponentAudioSource.h"
#include "ComponentReverbZone.h"


#include "ResourceMesh.h"

#include "GUICreator.h"
#include "ResourceMaterial.h"
#include "ResourceMesh.h"

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
	UUID = App->scene->GetNewUID();
	parentUUID = gameobject.parentUUID;
	isStatic = gameobject.isStatic;
	activeInHierarchy = gameobject.activeInHierarchy;
	activeSelf = gameobject.activeSelf;
	isVolumetric = gameobject.isVolumetric;
	hasLight = gameobject.hasLight;
	isBoneRoot = gameobject.isBoneRoot;

	assert(!(isVolumetric && hasLight));
	bbox = gameobject.bbox;
	navigable = gameobject.navigable;
	walkable = gameobject.walkable;
	noWalkable = gameobject.noWalkable;

	for (const auto& component : gameobject.components)
	{
		Component *componentcopy = component->Clone();
		componentcopy->gameobject = this;
		components.push_back(componentcopy);
		if (componentcopy->type == ComponentType::Transform)
		{
			transform = (ComponentTransform*)componentcopy;
		}
		if (componentcopy->type == ComponentType::Button)
		{
			((ComponentButton*)componentcopy)->text->gameobject = this;
			((ComponentButton*)componentcopy)->buttonImage->gameobject = this;
			((ComponentButton*)componentcopy)->highlightedImage->gameobject = this;
			((ComponentButton*)componentcopy)->pressedImage->gameobject = this;
			((ComponentButton*)componentcopy)->rectTransform->gameobject = this;
		}
	}
	if (!App->scene->photoEnabled)
	{
		if (GetComponent(ComponentType::Renderer) != nullptr || GetComponent(ComponentType::Light) != nullptr)
		{
			App->scene->AddToSpacePartition(this);
		}
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
	strcpy(go_name, name.c_str());
	ImGui::InputText("Name", go_name, MAX_NAME);
	name = go_name;
	delete[] go_name;

	if (this != App->scene->root)
	{
		if (ImGui::Checkbox("Active", &activeSelf))
		{
			SetActive(activeSelf);
		}

		ImGui::SameLine();
		//navigability
		if (isVolumetric && isStatic) 
		{
			if (ImGui::Checkbox("Navigable", &navigable))
			{
				App->navigation->navigableObjectToggled(this, navigable);
			}
			if (navigable)
			{
				//defines walls and this stuff
				ImGui::Checkbox("Walkable", &walkable);
				ImGui::Checkbox("No Walkable", &noWalkable);
			}
		}

		if (ImGui::Checkbox("Static", &isStatic))
		{
			if (isStatic && GetComponent(ComponentType::Renderer) != nullptr)
			{
				SetStaticAncestors();
				App->scene->dynamicGOs.erase(this);
				App->scene->staticGOs.insert(this);
				assert(!(hasLight && isVolumetric)); //Impossible combination
				if (hasLight && treeNode != nullptr)
				{
					App->spacePartitioning->aabbTreeLighting.ReleaseNode(treeNode);
				}
				if (isVolumetric && treeNode != nullptr)
				{
					App->spacePartitioning->aabbTree.ReleaseNode(treeNode);
				}
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

void GameObject::Update()
{
	if (!isActive()) return;

	for (auto& component : components)
	{
		if (component->enabled)
		{
			component->Update();
		}
	}
	//TESTING
	//---------------------------------------------
	if (isBoneRoot && App->time->gameState == GameState::RUN)
	{
		ComponentAnimation* compAnim = (ComponentAnimation*)GetComponent(ComponentType::Animation);
		if (App->input->GetKey(SDL_SCANCODE_X))
		{
			compAnim->SendTriggerToStateMachine("trigger1");
		}
		if (App->input->GetKey(SDL_SCANCODE_C))
		{
			compAnim->SendTriggerToStateMachine("trigger2");
		}
	}
	//---------------------------------------------

	for (const auto& child : children)
	{
		child->Update();
	}

	for (std::list<GameObject*>::iterator itChild = children.begin(); itChild != children.end();)
	{

		if ((*itChild)->copyFlag) //Moved GO
		{
			(*itChild)->copyFlag = false;
			GameObject *copy = new GameObject(**itChild);
			copy->parent = this;
			this->children.push_back(copy);
		}
		++itChild;
	}

}

void GameObject::SetActive(bool active)
{
	activeSelf = active;
	for (auto& child : children)
	{
		child->activeInHierarchy = active;
	}
}

Component* GameObject::CreateComponent(ComponentType type)
{
	Component* component = nullptr;
	switch (type)
	{
	case ComponentType::Transform:
		component = new ComponentTransform(this);
		this->transform = (ComponentTransform*)component;
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
			App->scene->lights.push_back((ComponentLight*)component);
			hasLight = true;
			light = (ComponentLight*)component;
			App->spacePartitioning->aabbTreeLighting.InsertGO(this);
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
		if (App->scene->maincamera == nullptr)
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
		component = new ComponentTransform2D(this);
		break;
	case ComponentType::Text:
		component = new ComponentText(this);
		break;
	case ComponentType::Image:
		component = new ComponentImage(this);
		break;
	case ComponentType::Button:
		component = new ComponentButton(this);
		break;
	case ComponentType::Script:
		component = new ComponentScript(this);
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
		if (App->audioManager->audioListeners.size() == 1)
		{
			App->audioManager->mainListener = (ComponentAudioListener*)component;
			App->audioManager->mainListener->isMainListener = true; 
		}
		break;
	case ComponentType::ReverbZone:
		component = new ComponentReverbZone(this);
		App->audioManager->reverbZones.push_back((ComponentReverbZone*)component);

		break;
	default:
		break;
	}
	if (component != nullptr)
	{
		components.push_back(component);
	}
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

Script* GameObject::GetScript() const
{
	ComponentScript* component = (ComponentScript*)GetComponent(ComponentType::Script);
	if (component != nullptr)
	{
		return component->GetScript();
	}
	return nullptr;
}

Script * GameObject::FindScriptByName(const char * name) const
{
	std::vector<Component*> components = GetComponents(ComponentType::Script);
	for (const auto& component : components)
	{
		if (((ComponentScript*)component)->GetScriptName() == name)
		{
			return ((ComponentScript*)component)->GetScript();
		}
	}
	return nullptr;
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

Component * GameObject::GetComponent(ComponentType type) const
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
	std::stack<const GameObject *>GOs;
	GOs.push(this);
	while (!GOs.empty())
	{
		const GameObject* go = GOs.top();
		GOs.pop();

		Component* component = go->GetComponent(type);
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

		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, App->renderer->shadowsTex);
		glUniform1i(glGetUniformLocation(shader, "shadowTex"), 5);
	}
}

AABB GameObject::GetBoundingBox() const
{
	return bbox;
}

bool GameObject::Intersects(const LineSegment & line, float &distance, math::float3* intersectionPoint) const
{
	LineSegment localLine(line);
	localLine.Transform(GetGlobalTransform().Inverted());
	ComponentRenderer* mesh_renderer = (ComponentRenderer*)GetComponent(ComponentType::Renderer);
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
	ComponentRenderer* renderer = (ComponentRenderer*)GetComponent(ComponentType::Renderer);
	if (renderer != nullptr)
	{
		if (renderer->mesh != nullptr)
			bbox = renderer->mesh->GetBoundingBox();

		bbox.TransformAsAABB(GetGlobalTransform());
	}
}

void GameObject::DrawBBox() const
{
	for (const auto& child : children)
	{
		child->DrawBBox();
	}

	ComponentRenderer *renderer = (ComponentRenderer*)GetComponent(ComponentType::Renderer);
	if (renderer == nullptr || renderer->mesh == nullptr) return;

	if (renderer->mesh->GetReferences() > 0u)
		renderer->mesh->DrawBbox(App->program->defaultShader->id[0], bbox);
}

bool GameObject::CleanUp()
{
	if (isStatic)
	{
		App->scene->quadtree->Remove(*this);
	}
	else
	{
		App->scene->dynamicGOs.erase(this);
	}

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

void GameObject::Save(JSON_value *gameobjects) const
{
	if (parent != nullptr && App->scene->canvas != this) // we don't add gameobjects without parent (ex: World)
	{
		JSON_value *gameobject = gameobjects->CreateValue();
		gameobject->AddUint("UID", UUID);
		gameobject->AddUint("ParentUID", parent->UUID);
		gameobject->AddString("Name", name.c_str());
		gameobject->AddUint("Static", isStatic);
		gameobject->AddUint("ActiveInHierarchy", activeInHierarchy);
		gameobject->AddUint("ActiveSelf", activeSelf);
		gameobject->AddUint("isBoneRoot", isBoneRoot);
		gameobject->AddFloat4x4("baseState", baseState);
		gameobject->AddUint("Navigable", navigable);
		gameobject->AddUint("Walkable", walkable);
		gameobject->AddUint("No Walkable", noWalkable);

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
		child->Save(gameobjects);
	}
}

void GameObject::Load(JSON_value *value)
{
	UUID = value->GetUint("UID");
	parentUUID = value->GetUint("ParentUID");
	name = value->GetString("Name");
	isStatic = value->GetUint("Static");
	activeInHierarchy = value->GetUint("ActiveInHierarchy", 1);
	activeSelf = value->GetUint("ActiveSelf", 1);
	isBoneRoot = value->GetUint("isBoneRoot");
	baseState = value->GetFloat4x4("baseState");
	navigable = value->GetUint("Navigable");
	walkable = value->GetUint("Walkable");
	noWalkable = value->GetUint("No Walkable");

	JSON_value* componentsJSON = value->GetValue("Components");
	for (unsigned i = 0; i < componentsJSON->Size(); i++)
	{
		JSON_value* componentJSON = componentsJSON->GetValue(i);
		ComponentType type = (ComponentType)componentJSON->GetUint("Type");
		Component* component = CreateComponent(type);
		component->Load(componentJSON);
	}

	if (transform != nullptr)
	{
		transform->UpdateTransform();
	}

	if (hasLight)
	{
		transform->UpdateTransform();
		ComponentLight* light = (ComponentLight*)GetComponent(ComponentType::Light);
		if (light->lightType == LightType::DIRECTIONAL)
		{
			App->renderer->directionalLight = light;
		}
	}
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
	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen
		| ImGuiTreeNodeFlags_OpenOnDoubleClick | (isSelected ? ImGuiTreeNodeFlags_Selected : 0);

	ImGui::PushID(this);
	if (children.empty())
	{
		node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	}
	ImGui::Text("|"); ImGui::SameLine();
	App->scene->DragNDropMove(this);
	bool obj_open = ImGui::TreeNodeEx(this, node_flags, name.c_str());
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

	if (obj_open)
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

		if (go->GetComponent(ComponentType::Renderer) != nullptr)
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

void GameObject::UpdateTransforms(math::float4x4 parentGlobal)
{
	PROFILE;
	if (movedFlag && transform)
	{
		transform->local = math::float4x4::FromTRS(transform->position, transform->rotation, transform->scale);
		movedFlag = false;
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

	for (const auto& child : children)
	{
		child->movedFlag = true;
		child->UpdateTransforms(global);
	}

	UpdateBBox();
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
