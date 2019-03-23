#include "ComponentTransform2D.h"
#include "Application.h"
#include "GameObject.h"
#include "ModuleTime.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "JSON.h"

ComponentTransform2D::ComponentTransform2D(GameObject* gameobject) : Component(gameobject, ComponentType::Transform2D)
{
}

ComponentTransform2D::ComponentTransform2D(const ComponentTransform2D & component) : Component(component)
{
	position = component.position;
	size = component.size;
}

ComponentTransform2D::~ComponentTransform2D()
{
}

Component * ComponentTransform2D::Clone() const
{
	return new ComponentTransform2D(*this);
}

void ComponentTransform2D::DrawProperties()
{

	if (ImGui::CollapsingHeader("Rect Transformation", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (gameobject->isStatic && App->time->gameState != GameState::RUN)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		ImGui::DragFloat2("Position", (float*)&position, 0.1f, -1000.f, 1000.f);

		ImGui::DragFloat2("Size", (float*)&size, 0.5f, 0.f, 1000.f);
	
		if (gameobject->isStatic && App->time->gameState != GameState::RUN)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}
}

void ComponentTransform2D::Save(JSON_value * value) const
{
	Component::Save(value);
	value->AddFloat2("Position", position);
	value->AddFloat2("Size", size);
}

void ComponentTransform2D::Load(JSON_value* value)
{
	Component::Load(value);
	position = value->GetFloat2("Position");
	size = value->GetFloat2("Size");
}