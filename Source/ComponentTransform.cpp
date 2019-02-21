#include "Application.h"
#include "ComponentTransform.h"
#include "GameObject.h"
#include "ModuleTime.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "Math/MathFunc.h"
#include "JSON.h"


ComponentTransform::ComponentTransform(GameObject* gameobject, const float4x4 &transform) : Component(gameobject, ComponentType::Transform)
{
	AddTransform(transform);
}

ComponentTransform::ComponentTransform(const ComponentTransform & component) : Component(component)
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

void ComponentTransform::AddTransform(const float4x4 & transform)
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
	
	if (ImGui::CollapsingHeader("Local Transformation", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (gameobject->isStatic && App->time->gameState != GameState::RUN)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		ImGui::DragFloat3("Position", (float*)&position, 0.1f, -1000.f, 1000.f);

		ImGui::DragFloat3("Rotation", (float*)&eulerRotation, 0.5f, -180, 180.f);

		rotation = rotation.FromEulerXYZ(math::DegToRad(eulerRotation.x),
			math::DegToRad(eulerRotation.y), math::DegToRad(eulerRotation.z));

		ImGui::DragFloat3("Scale", (float*)&scale, 0.1f, 0.01f, 100.f);
		ImGui::Separator();

		if (gameobject->isStatic && App->time->gameState != GameState::RUN)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

		if (old_position != position || old_euler != eulerRotation || old_scale != scale)
		{
			UpdateTransform();
			gameobject->moved_flag = true;
			gameobject->UpdateBBox();
		}
	}
}

void ComponentTransform::UpdateTransform()
{
	UpdateOldTransform();
	global = global * local.Inverted();
	local = float4x4::FromTRS(position, rotation, scale);
	global = global * local;
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

void ComponentTransform::SetLocalToWorld()
{
	local = global;
	global.Decompose(position, rotation, scale);
	RotationToEuler();
	UpdateOldTransform();
	RotationToEuler();
}

void ComponentTransform::SetWorldToLocal(const float4x4 & newparentGlobalMatrix)
{
	local = newparentGlobalMatrix.Inverted() * local;
	local.Decompose(position, rotation, scale);
	RotationToEuler();
	UpdateOldTransform();
	global = newparentGlobalMatrix * local;
	RotationToEuler();
}

void ComponentTransform::SetGlobalTransform(const float4x4 & newglobal, const float4x4 &parentglobal)
{
	global = newglobal;
	local = parentglobal.Inverted() * global;
	local.Decompose(position, rotation, scale);
	RotationToEuler();
	UpdateOldTransform();
}

float3 ComponentTransform::GetGlobalPosition()
{
	return global.Col3(3);
}

void ComponentTransform::Save(JSON_value * value) const
{
	Component::Save(value);
	value->AddFloat3("Position", position);
	value->AddQuat("Rotation", rotation);
	value->AddFloat3("Euler", eulerRotation);
	value->AddFloat3("Scale", scale);
	value->AddFloat4x4("Global", global);
}

void ComponentTransform::Load(const JSON_value & value)
{
	Component::Load(value);
	position = value.GetFloat3("Position");
	rotation = value.GetQuat("Rotation");
	eulerRotation = value.GetFloat3("Euler");
	scale = value.GetFloat3("Scale");
	global = value.GetFloat4x4("Global");
	local = float4x4::FromTRS(position, rotation, scale);
	RotationToEuler();
}
