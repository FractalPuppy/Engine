#include "GameObject.h"

#include "Application.h"
#include "ModuleProgram.h"
#include "ModuleEditor.h"
#include "ModuleCamera.h"
#include "ModuleInput.h"
#include "ModuleScene.h"
#include "ModuleTextures.h"
#include "ModuleRender.h"
#include "ModuleSpacePartitioning.h"

#include "Component.h"
#include "ComponentTransform.h"
#include "ComponentCamera.h"
#include "ComponentLight.h"
#include "ComponentRenderer.h"
#include "ComponentTransform2D.h"
#include "ComponentText.h"
#include "ComponentImage.h"
#include "ComponentButton.h"
#include "ComponentScript.h"

#include "ResourceMesh.h"

#include "GUICreator.h"
#include "myQuadTree.h"
#include "AABBTree.h"
#include <stack>
#include "JSON.h"

#include "Math/float4x4.h"
#include "Math/MathFunc.h"
#include "Geometry/LineSegment.h"
#include "GL/glew.h"
#include "imgui.h"

#define MAX_NAME 64
#define IMGUI_RIGHT_MOUSE_BUTTON 1

GameObject::GameObject(const char * name, unsigned uuid) : name(name), UUID(uuid)
{
}

GameObject::GameObject(const float4x4 & transform, const char * name, unsigned uuid) : name(name), UUID(uuid)
{
	this->transform =  (ComponentTransform*) CreateComponent(ComponentType::Transform);
	this->transform->AddTransform(transform);
}

GameObject::GameObject(const GameObject & gameobject)
{
	name = gameobject.name;
	UUID = App->scene->GetNewUID();
	parentUUID = gameobject.parentUUID;
	isStatic = gameobject.isStatic;
	bbox = gameobject.bbox;

	for (const auto& component: gameobject.components)
	{
		Component *componentcopy = component->Clone();
		componentcopy->gameobject = this;
		components.push_back(componentcopy);
		if (componentcopy->type == ComponentType::Transform)
		{
			transform = (ComponentTransform*)componentcopy;
		}
	}
	if (!App->scene->photoEnabled)
	{
		if (GetComponent(ComponentType::Renderer) != nullptr)
		{
			isVolumetric = true;
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
	Component* button = GetComponent(ComponentType::Button); //ESTO LO TIENE QUE HACER EL CANVAAS RECORRIENDO SUS HIJOS / ES DE PRUEBA
	if (button != nullptr)
	{
		button->Update();
	}

	for (auto& component: components)
	{
		component->Update();
	}
	for (std::list<GameObject*>::iterator itChild = children.begin(); itChild != children.end();)
	{

		if ((*itChild)->copyFlag) //Moved GO
		{
			(*itChild)->copyFlag = false;
			GameObject *copy = new GameObject(**itChild);
			copy->parent = this;
			copy->isVolumetric = (*itChild)->isVolumetric;
			copy->hasLight = (*itChild)->hasLight;
			assert(!(copy->isVolumetric && copy->hasLight)); //incompatible component configuration
			if (copy->isVolumetric)
			{
				if (isStatic)
				{
					App->scene->staticGOs.insert(copy);
				}
				else
				{
					App->spacePartitioning->aabbTree.InsertGO(copy);
				}
			}
			if (copy->hasLight)
			{
				for (Component* component : copy->components)
				{
					if (component->type == ComponentType::Light)
					{
						copy->light = (ComponentLight*)component;
					}
				}
				App->spacePartitioning->aabbTreeLighting.InsertGO(copy);
				App->scene->lights.push_back(copy->light);
			}
			copy->transform->SetPosition(copy->transform->GetPosition() + copy->transform->front);
			copy->transform->UpdateTransform();
			this->children.push_back(copy);
		}
		if ((*itChild)->movedFlag) //Moved GO
		{
			for (auto child : (*itChild)->children)
			{
				child->UpdateGlobalTransform();
			}
			(*itChild)->UpdateBBox();
			(*itChild)->movedFlag = false;
		}

		(*itChild)->Update(); //Update after moved_flag check

		if ((*itChild)->copyFlag) //Copy GO
		{
			(*itChild)->copyFlag = false;
			GameObject *copy = new GameObject(**itChild);
			copy->parent = this;
			this->children.push_back(copy);
		}
		if ((*itChild)->deleteFlag) //Delete GO
		{
			(*itChild)->deleteFlag = false;
			(*itChild)->CleanUp();
			App->scene->DeleteFromSpacePartition(*itChild);
			delete *itChild;
			children.erase(itChild++);
		}
		else
		{
			++itChild;
		}
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
	std::stack<const GameObject *>GOs;
	GOs.push(this);
	while (!GOs.empty())
	{
		const GameObject* go = GOs.top();
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

void GameObject::RemoveComponent(const Component & component)
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

void GameObject::UpdateGlobalTransform() //Updates global transform when moving
{
	float4x4 mytransform = GetLocalTransform();
	if (parent != nullptr)
	{
		mytransform = parent->GetGlobalTransform() * mytransform;
	}
	if (transform != nullptr)
	{
		transform->global = mytransform;
	}
	UpdateBBox();

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
	unsigned directionals = 0u;
	unsigned points = 0u;
	unsigned spots = 0u;
	char buffer[32];
	//LOG("%s got %d lights", name.c_str(), lights.size());
	for (GameObject* go : lights)
	{
		assert(go->light != nullptr);
		switch (go->light->lightType)
		{
		case LightType::DIRECTIONAL:
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
}

AABB GameObject::GetBoundingBox() const
{
	return bbox;
}

bool GameObject::Intersects(const LineSegment & line, float &distance) const
{
	LineSegment localLine(line);
	localLine.Transform(GetGlobalTransform().Inverted());
	ComponentRenderer* mesh_renderer = (ComponentRenderer*)GetComponent(ComponentType::Renderer);
	if (mesh_renderer != nullptr)
	{
		if (mesh_renderer->mesh->Intersects(localLine, &distance))
		{
			return true;
		}
	}
	return false;
}

void GameObject::UpdateBBox()
{
	ComponentRenderer* renderer = (ComponentRenderer*) GetComponent(ComponentType::Renderer);
	if (renderer != nullptr)
	{
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
	if (renderer == nullptr) return;

	if(renderer->mesh->GetReferences() > 0u)
		renderer->mesh->DrawBbox(App->program->defaultShader->id, bbox);
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

	JSON_value* componentsJSON = value->GetValue("Components");
	for (unsigned i = 0; i < componentsJSON->Size(); i++)
	{
		JSON_value* componentJSON = componentsJSON->GetValue(i);
		ComponentType type = (ComponentType) componentJSON->GetUint("Type");
		Component* component = CreateComponent(type);
		component->Load(componentJSON);
	}

	if (transform != nullptr)
	{
		transform->UpdateTransform();
	}

	if (hasLight)
	{
		if (treeNode != nullptr)
		{
			App->spacePartitioning->aabbTreeLighting.ReleaseNode(treeNode);
		}
		light->CalculateGuizmos();
		App->spacePartitioning->aabbTreeLighting.InsertGO(this);
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