#include "Component.h"
#include "GameObject.h"
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
}

Component::~Component()
{
	gameobject = nullptr;
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

void Component::Remove()
{
	gameobject->RemoveComponent(*this);
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
