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

#include "GUICreator.h"
#include "Material.h"
#include "Mesh.h"
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

	if (GetComponent(ComponentType::Renderer) != nullptr)
	{
		isVolumetric = true;
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
				SetStaticAncestors(); //TODO: Propagate staticness & update aabbtree
				App->scene->dynamicGOs.erase(this);
				App->scene->staticGOs.insert(this);
				App->spacePartitioning->kDTree.Calculate();
			}
			else if (!isStatic)
			{
				//TODO: Propagate staticness & update aabbtree
				App->scene->dynamicGOs.insert(this);
				App->scene->staticGOs.erase(this);
				App->spacePartitioning->kDTree.Calculate();
				App->spacePartitioning->aabbTree.InsertGO(this); //TODO: remove this when propagation is corrected 
			}
		}
	}

	for (auto &component : components)
	{
		component->DrawProperties();
	}
}

void GameObject::Update()
{
	for (auto& component: components)
	{
		component->Update();
	}
	for (std::list<GameObject*>::iterator it_child = children.begin(); it_child != children.end();)
	{
		(*it_child)->Update();

		if ((*it_child)->copy_flag) //Copy GO
		{
			(*it_child)->copy_flag = false;
			GameObject *copy = new GameObject(**it_child);
			copy->parent = this;
			this->children.push_back(copy);
		}
		if ((*it_child)->moved_flag) //Moved GO
		{
			for (auto child : (*it_child)->children)
			{
				child->UpdateGlobalTransform();
			}
			(*it_child)->UpdateBBox();
			(*it_child)->moved_flag = false;
		}
		if ((*it_child)->delete_flag) //Delete GO
		{
			(*it_child)->delete_flag = false;
			(*it_child)->CleanUp();
			delete *it_child;
			children.erase(it_child++);
		}
		else
		{
			++it_child;
		}
	}
}

Component * GameObject::CreateComponent(ComponentType type)
{
	Component* component = nullptr;
	switch (type)
	{
	case ComponentType::Transform:
		component = new ComponentTransform(this);
		this->transform = (ComponentTransform*)component;
		break;
	case ComponentType::Renderer:
		component = new ComponentRenderer(this);		
		break;
	case ComponentType::Light:
		component = new ComponentLight(this);
		App->scene->lights.push_back((ComponentLight*)component);
		break;
	case ComponentType::Camera:
		component = new ComponentCamera(this);
		if (App->scene->maincamera == nullptr)
		{
			App->scene->maincamera = (ComponentCamera*)component;
			App->scene->maincamera->isMainCamera = true;
		}
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
	for (std::vector<Component*>::iterator it = components.begin(); it != components.end(); ++it)
	{
		if (*it == &component)
		{
			(*it)->CleanUp();
			components.erase(it);
			RELEASE(*it);
			return;
		}
	}
}

void GameObject::RemoveChild(GameObject* bastard)
{
	children.remove(bastard);
	RELEASE(bastard);
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
	transform->global = mytransform;
	UpdateBBox();

	for (auto &child : children)
	{
		child->UpdateGlobalTransform();
	}
}

void GameObject::SetGlobalTransform(const float4x4 & global) //Replaces global transform
{
	moved_flag = true;
	if (transform != nullptr)
	{
		float4x4 parentglobal = float4x4::identity;
		if (parent != nullptr && parent->transform != nullptr)
		{
			parentglobal = parent->transform->global;
		}
		transform->SetGlobalTransform(global, parentglobal);
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
	ComponentLight* directional = App->scene->GetDirectionalLight();
	if (directional != nullptr)
	{
		glUniform3fv(glGetUniformLocation(shader,
			"lights.directional.direction"), 1, (GLfloat*)&directional->direction);

		glUniform3fv(glGetUniformLocation(shader,
			"lights.directional.color"), 1, (GLfloat*)&directional->color);
	}
	else
	{
		float3 noDirectional = float3::zero;
		glUniform3fv(glGetUniformLocation(shader,
			"lights.directional.direction"), 1, (GLfloat*)&noDirectional);
	}


	int i = 0;
	for (const auto & spot : App->scene->GetClosestLights(LightType::SPOT, transform->position))
	{
		char buffer[32];

		sprintf(buffer, "lights.spots[%d].position", i);
		glUniform3fv(glGetUniformLocation(shader,
			buffer), 1, (GLfloat*)&spot->position);

		memset(buffer, 0, 32);
		sprintf(buffer, "lights.spots[%d].direction", i);
		glUniform3fv(glGetUniformLocation(shader,
			buffer), 1, (GLfloat*)&spot->direction);

		memset(buffer, 0, 32);
		sprintf(buffer, "lights.spots[%d].color", i);
		glUniform3fv(glGetUniformLocation(shader,
			buffer), 1, (GLfloat*)&spot->color);

		memset(buffer, 0, 32);
		sprintf(buffer, "lights.spots[%d].attenuation", i);
		glUniform3fv(glGetUniformLocation(shader,
			buffer), 1, (GLfloat*)&spot->attenuation);

		memset(buffer, 0, 32);
		float innerRad = cosf(math::DegToRad(spot->inner));
		sprintf(buffer, "lights.spots[%d].inner", i);
		glUniform1fv(glGetUniformLocation(shader,
			buffer), 1, (GLfloat*)&innerRad);

		memset(buffer, 0, 32);
		float outerRad = cosf(math::DegToRad(spot->outer));
		sprintf(buffer, "lights.spots[%d].outer", i);
		glUniform1fv(glGetUniformLocation(shader,
			buffer), 1, (GLfloat*)&outerRad);

		++i;
	}
	glUniform1i(glGetUniformLocation(shader,
		"lights.num_spots"), i);

	i = 0;
	for (const auto & point : App->scene->GetClosestLights(LightType::POINT, transform->position))
	{
		char buffer[32];

		sprintf(buffer, "lights.points[%d].position", i);
		glUniform3fv(glGetUniformLocation(shader,
			buffer), 1, (GLfloat*)&point->position);

		memset(buffer, 0, 32);
		sprintf(buffer, "lights.points[%d].direction", i);
		glUniform3fv(glGetUniformLocation(shader,
			buffer), 1, (GLfloat*)&point->direction);

		memset(buffer, 0, 32);
		sprintf(buffer, "lights.points[%d].color", i);
		glUniform3fv(glGetUniformLocation(shader,
			buffer), 1, (GLfloat*)&point->color);

		memset(buffer, 0, 32);
		sprintf(buffer, "lights.points[%d].attenuation", i);
		glUniform3fv(glGetUniformLocation(shader,
			buffer), 1, (GLfloat*)&point->attenuation);

		++i;
	}
	glUniform1i(glGetUniformLocation(shader,
		"lights.num_points"), i);
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
	if (parent != nullptr) // we don't add gameobjects without parent (ex: World)
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
		component->Load(*componentJSON);
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

void GameObject::DrawHierarchy(GameObject * selected)
{
	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen
		| ImGuiTreeNodeFlags_OpenOnDoubleClick | (selected == this ? ImGuiTreeNodeFlags_Selected : 0);

	ImGui::PushID(this);
	if (children.empty())
	{
		node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	}
	bool obj_open = ImGui::TreeNodeEx(this, node_flags, name.c_str());
	if (ImGui::IsItemClicked())
	{
		App->scene->Select(this);
	}
	App->scene->DragNDrop(this);
	if (ImGui::IsItemHovered() && App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_DOWN)
	{
		ImGui::OpenPopup("gameobject_options_popup");
	}
	if (ImGui::BeginPopup("gameobject_options_popup"))
	{
		GUICreator::CreateElements(this);
		if (ImGui::Selectable("Duplicate"))
		{
			copy_flag = true;
		}
		if (ImGui::Selectable("Delete"))
		{
			delete_flag = true;
			if (selected == this)
			{
				App->scene->selected = nullptr;
			}
		}
		ImGui::EndPopup();
	}
	if (obj_open)
	{
		for (auto &child : children)
		{
			child->DrawHierarchy(selected);
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
			if (go->treeNode != nullptr)
				App->spacePartitioning->aabbTree.ReleaseNode(go->treeNode);

			App->scene->quadtree->Insert(go);
		}
		parents.pop();
		parents.push(go->parent);
	}
}