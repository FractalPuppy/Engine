#include "PanelNavigation.h"

#include "Application.h"
#include "GameObject.h"

#include "ModuleNavigation.h"
#include "ModuleScene.h"

#include "imgui.h"


PanelNavigation::PanelNavigation()
{
}


PanelNavigation::~PanelNavigation()
{
}


void PanelNavigation::Draw()
{
	if (App->scene->selected != nullptr && App->scene->selected->navigable)
	{
		if (!ImGui::Begin("Navigation", &enabled))
		{
			ImGui::End();
			return;
		}
		App->navigation->DrawGUI();
		ImGui::End();
	}
}