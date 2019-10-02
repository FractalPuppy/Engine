#include "PanelHierarchy.h"
#include "Application.h"
#include "ModuleScene.h"
#include "GameObject.h"
#include "imgui.h"

PanelHierarchy::PanelHierarchy()
{
}


PanelHierarchy::~PanelHierarchy()
{
}

void PanelHierarchy::Draw()
{
	if (!ImGui::Begin("Hierarchy", &enabled, ImGuiWindowFlags_HorizontalScrollbar))
	{
		ImGui::End();
		return;
	}
	App->scene->DrawHierarchy();
	focus = ImGui::IsWindowFocused();
	hover = ImGui::IsWindowHovered();
	ImGui::End();
}