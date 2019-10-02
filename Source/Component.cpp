#include "Application.h"
#include "ModuleScene.h"
#include "Component.h"
#include "ComponentTransform.h"
#include "GameObject.h"
#include "ModuleScene.h"
#include "ModuleRender.h"
#include "Application.h"
#include "imgui.h"
#include "JSON.h"

Component::Component(GameObject* gameobject, ComponentType type): gameobject(gameobject), type(type)
{
}

Component::Component(const Component & component)
{
	gameobject = component.gameobject;
	type = component.type;
	enabled = component.enabled;
	//gameobject->transform->SetGlobalPosition(gameobject->transform->GetGlobalPosition() + gameobject->transform->front * App->renderer->current_scale);
}

Component::~Component()
{
	gameobject = nullptr;
	OnDisable();
	OnDestroy();
}

void Component::Enable(bool enable)
{
	enabled = enable;
	if (enable && gameobject->isActive())
	{
		OnEnable();
	}
	else if(!enable)
	{
		OnDisable();
	}
}

bool Component::DrawComponentState()
{
	ImGui::Checkbox("Active", &enabled); ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.f, 0.6f, 0.6f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.f / 7.0f, 0.7f, 0.7f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.f / 7.0f, 0.8f, 0.8f));
	
	bool removed = ImGui::Button("Remove");
	if(removed) Remove();
	
	ImGui::PopStyleColor(3);
	return removed;
}

void Component::Options()
{
	ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize("Opt   ").x);
	if (ImGui::Button("Opt"))
	{
		ImGui::OpenPopup("Options");
	}

	const char* options[] = { "Copy Component Values", "Paste Component Values", "Reset" };

	if (ImGui::BeginPopup("Options"))
	{
		for (int i = 0; i < IM_ARRAYSIZE(options); i++)
			if (ImGui::Selectable(options[i]))
			{
				if (i == 0) // Copy
				{
					Copy();
				}
				else if (i == 1) // Paste
				{
					Paste();
				}
				else if (i == 2) // Reset
				{
					Reset();
				}
			}
		ImGui::EndPopup();
	}
}

void Component::Remove()
{
	App->scene->TakePhoto();
	gameobject->RemoveComponent(*this);
}

void Component::Copy()
{
	App->scene->copyComp = Clone();
}

void Component::Save(JSON_value * value) const
{
	assert(value != nullptr);
	if (value == nullptr) return;
	value->AddUint("Type", (unsigned)type);
	value->AddInt("Enabled", enabled);
}

void Component::Load(JSON_value* value)
{
	type = (ComponentType) value->GetUint("Type");
	enabled = (bool) value->GetInt("Enabled");
}
