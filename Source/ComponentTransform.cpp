#include "Application.h"
#include "ComponentTransform.h"

#include "Application.h"
#include "ModuleSpacePartitioning.h"
#include "ModuleInput.h"
#include "ModuleWindow.h"
#include "ModuleTime.h"
#include "ModuleScene.h"
#include "ModuleUI.h"

#include "GameObject.h"
#include "ComponentLight.h"
#include "ComponentCamera.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "Math/MathFunc.h"
#include "Math/float2.h"
#include "Geometry/Plane.h"
#include "Geometry/Line.h"
#include "JSON.h"
#include "AABBTree.h"


ComponentTransform::ComponentTransform(GameObject* gameobject, const math::float4x4 &transform) : Component(gameobject, ComponentType::Transform)
{
	AddTransform(transform);
}

ComponentTransform::ComponentTransform(const ComponentTransform& component) : Component(component)
{
	position = component.position;
	rotation = component.rotation;
	eulerRotation = component.eulerRotation;
	scale = component.scale;
	local = component.local;
	global = component.global;
}


ComponentTransform::~ComponentTransform()
{
	if (gameobject != nullptr)
	{
		gameobject->transform = nullptr;
	}
}

Component * ComponentTransform::Clone() const
{
	return new ComponentTransform(*this);
}

void ComponentTransform::AddTransform(const math::float4x4& transform)
{
	transform.Decompose(position, rotation, scale);
	RotationToEuler();
	local = transform;
	GameObject* parent = gameobject->parent;
	if (parent != nullptr && parent->transform != nullptr)
	{
		global = parent->transform->global * local;
	}
}

void ComponentTransform::DrawProperties()
{
	ImGui::PushID(this);

	if (ImGui::CollapsingHeader("Local Transformation", ImGuiTreeNodeFlags_DefaultOpen))
	{
		Options();

		if (gameobject->isStatic && App->time->gameState != GameState::RUN)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		ImGui::DragFloat3("Position", (float*)&position, 0.1f, (float) App->scene->SceneSize * -1, (float)App->scene->SceneSize);
		ImGui::DragFloat3("Rotation", (float*)&eulerRotation, 0.5f, -180, 180.f);
		ImGui::DragFloat3("Scale", (float*)&scale, 0.1f, 0.01f, 100.f);
		rotation = rotation.FromEulerXYZ(math::DegToRad(eulerRotation.x),
			math::DegToRad(eulerRotation.y), math::DegToRad(eulerRotation.z));
		
		ImGui::Separator();

		if (gameobject->isStatic && App->time->gameState != GameState::RUN)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

		if (old_position != position || old_euler != eulerRotation || old_scale != scale)
		{
			UpdateTransform();
			gameobject->movedFlag = true;
			if (App->scene->photoTimer <= 0.f)
			{
				App->scene->TakePhoto();
			}
		}
	}
	ImGui::PopID();
}


void ComponentTransform::MultiSelectionTransform(float4x4 &difference)
{
	for (GameObject* go : App->scene->selection)
	{
		if (go != App->scene->selected)
		{
			go->SetGlobalTransform(go->transform->global + difference);
			if (go->parent->transform != nullptr)
			{
				go->transform->local = go->parent->transform->global.Inverted().Mul(go->transform->global);
			}
			else
			{
				go->transform->local = go->transform->global;
			}
		}
	}
}

void ComponentTransform::UpdateTransform()
{
	UpdateOldTransform();
	
	front = -global.Col3(2);
	up = global.Col3(1);
	right = global.Col3(0);

}

void ComponentTransform::RotationToEuler()
{
	eulerRotation = rotation.ToEulerXYZ();
	eulerRotation.x = math::RadToDeg(eulerRotation.x);
	eulerRotation.y = math::RadToDeg(eulerRotation.y);
	eulerRotation.z = math::RadToDeg(eulerRotation.z);
}

void ComponentTransform::UpdateOldTransform()
{
	old_position = position;
	old_euler = eulerRotation;
	old_scale = scale;	
}

void ComponentTransform::UpdateGlobalTransform()
{
	if(gameobject->parent != nullptr)
		global = gameobject->parent->GetGlobalTransform() * local;
}

void ComponentTransform::SetLocalToWorld()
{
	local = global;
	global.Decompose(position, rotation, scale);
	RotationToEuler();
	UpdateOldTransform();
	RotationToEuler();
}

void ComponentTransform::SetWorldToLocal(const math::float4x4& newparentGlobalMatrix)
{
	local = newparentGlobalMatrix.Inverted() * local;
	local.Decompose(position, rotation, scale);
	RotationToEuler();
	UpdateOldTransform();
	global = newparentGlobalMatrix * local;
	RotationToEuler();
}

void ComponentTransform::SetGlobalTransform(const math::float4x4& newglobal, const math::float4x4& parentglobal)
{
	global = newglobal;
	local = parentglobal.Inverted() * global;
	local.Decompose(position, rotation, scale);
	RotationToEuler();
	UpdateOldTransform();
	
	if (position.Abs().x >= App->scene->SceneSize)
	{
		position.x = position.x / position.Abs().x * App->scene->SceneSize;
	}
	if (position.Abs().y >= App->scene->SceneSize)
	{
		position.y = position.y / position.Abs().y * App->scene->SceneSize;
	}
	if (position.Abs().z >= App->scene->SceneSize)
	{
		position.z = position.z / position.Abs().z * App->scene->SceneSize;
	}

	front = -global.Col3(2);
	up = global.Col3(1);
	right = global.Col3(0);

	if (!gameobject->isStatic)
	{
		if (gameobject->treeNode != nullptr && gameobject->hasLight)
		{
			gameobject->light->CalculateGuizmos();
			if (!gameobject->treeNode->aabb.Contains(gameobject->bbox))
			{
				App->spacePartitioning->aabbTreeLighting.ReleaseNode(gameobject->treeNode);
				App->spacePartitioning->aabbTreeLighting.InsertGO(gameobject);
			}
		}
		if (gameobject->treeNode != nullptr && gameobject->isVolumetric)
		{
			if (!gameobject->treeNode->aabb.Contains(gameobject->bbox))
			{
				App->spacePartitioning->aabbTree.ReleaseNode(gameobject->treeNode);
				App->spacePartitioning->aabbTree.InsertGO(gameobject);
			}
		}
	}
	else
	{
		App->spacePartitioning->kDTree.Calculate();
	}
}

void ComponentTransform::SetLocalTransform(const math::float4x4& newLocal, const math::float4x4& parentGlobal)
{
	local = newLocal;
	
	//global = parentGlobal.Mul(local);
	local.Decompose(position, rotation, scale);
	RotationToEuler();
}

void ComponentTransform::SetPosition(const math::float3 & newPosition)
{
	position = newPosition;
	gameobject->movedFlag = true;
	
}

math::float3 ComponentTransform::GetPosition()
{
	return position;
}

math::float2 ComponentTransform::GetScreenPosition()
{
	math::float3 projection = App->scene->maincamera->frustum->Project(GetGlobalPosition());
	if (projection.z > 0)
	{
		math::float2 screenPosition;
		screenPosition.x = (int)(projection.x * (App->ui->currentWidth / 2.0));
		screenPosition.y = (int)(projection.y * (App->ui->currentHeight / 2.0));
		return screenPosition;
	}
	return math::float2::zero;
}

void ComponentTransform::SetRotation(const math::Quat & newRotation)
{
	rotation = newRotation;
	RotationToEuler();
	gameobject->movedFlag = true;
	UpdateTransform();
}

ENGINE_API void ComponentTransform::Scale(float scalar)
{
	scale *= scalar;
	gameobject->movedFlag = true;
}

math::Quat ComponentTransform::GetRotation()
{
	return rotation;
}

math::float3 ComponentTransform::GetGlobalPosition()
{
	if (gameobject->movedFlag)
	{
		float4x4 newlocal = math::float4x4::FromTRS(position, rotation, scale);
		if (gameobject->parent != nullptr)
		{
			return (gameobject->parent->GetGlobalTransform() * newlocal).Col3(3);
		}
		return newlocal.Col3(3);
	}
	return global.Col3(3);
}

void ComponentTransform::LookAt(const math::float3 & targetPosition)
{
	math::float3 direction = (targetPosition - GetGlobalPosition());
	math::Quat newRotation = rotation.LookAt(float3::unitZ, direction.Normalized(), float3::unitY, float3::unitY);	
	SetRotation(newRotation);
}

ENGINE_API void ComponentTransform::LookAtMouse() //POS SIZE!!
{
	//App->renderer->gam
	math::float2 pos= App->window->GetWindowPos();
	math::float2 size = App->window->GetWindowSize();

	math::float2 mouse((float*)&App->input->GetMousePosition());

	float normalized_x = ((mouse.x - pos.x) /size.x) * 2 - 1; //0 to 1 -> -1 to 1
	float normalized_y = (1 - (mouse.y - pos.y) / size.y) * 2 - 1; //0 to 1 -> -1 to 1

	LineSegment segment = App->scene->maincamera->DrawRay(normalized_x, normalized_y);
	Line line(segment);
	Plane plane(position, float3::unitY);

	float d = 0;
	if (!line.Intersects(plane, &d))
	{
		assert(true, "Mouse didn't intersect with gameobject base plane!");
	}
	if (&d != nullptr)
	{
		math::float3 point = line.GetPoint(d);
		LookAt(point);
	}
}

void ComponentTransform::Align(const math::float3 & targetFront)
{
	Quat newRotation = Quat::RotateFromTo(front.Normalized(), targetFront.Normalized());
	SetRotation(rotation.Mul(newRotation));
}

void ComponentTransform::Save(JSON_value* value) const
{
	Component::Save(value);
	value->AddFloat3("Position", position);
	value->AddQuat("Rotation", rotation);
	value->AddFloat3("Euler", eulerRotation);
	value->AddFloat3("Scale", scale);
	value->AddFloat4x4("Global", global);
}

void ComponentTransform::Load(JSON_value* value)
{
	Component::Load(value);
	position = value->GetFloat3("Position");
	rotation = value->GetQuat("Rotation");
	eulerRotation = value->GetFloat3("Euler");
	scale = value->GetFloat3("Scale");
	global = value->GetFloat4x4("Global");
	local = math::float4x4::FromTRS(position, rotation, scale);
	RotationToEuler();
}

/*void ComponentTransform::Copy()
{
	App->scene->copyComp = Clone();
}*/

void ComponentTransform::Paste()
{
	if (App->scene->copyComp != nullptr && App->scene->copyComp->type == this->type)
	{
		ComponentTransform* comp = (ComponentTransform*)App->scene->copyComp;
		position = comp->position;
		eulerRotation = comp->eulerRotation;
		scale = comp->scale;
	}
}

void ComponentTransform::Reset()
{
	math::float3 position = math::float3::zero;
	rotation = math::Quat::identity;
	math::float3 eulerRotation = math::float3::zero;
	math::float3 scale = math::float3::one;
	math::float4x4 local = math::float4x4::identity;
	math::float4x4 animatedLocal = math::float4x4::identity;
	math::float4x4 global = math::float4x4::identity;
}
