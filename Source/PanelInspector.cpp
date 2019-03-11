#include "Application.h"

#include "ModuleScene.h"
#include "ModuleEditor.h"

#include "GameObject.h"
#include "Component.h"

#include "PanelInspector.h"
#include "PanelHierarchy.h"
#include "imgui.h"

#define WARNINGPOPUP "WarningPopup"
#define MAXCOMPONENTS "This GameObject cannot have this component twice! \n\n"
#define WORLDGO "Cannot add components to World gameobject"

PanelInspector::PanelInspector()
{
}


PanelInspector::~PanelInspector()
{
}

void PanelInspector::Draw()
{
	if (focus)
	{
		focus = false;
		ImGui::SetNextWindowFocus();
	}
	if (!ImGui::Begin("Inspector", &enabled))
	{
		ImGui::End();
		return;
	}
	ImGui::PushID(this);
	if (App->scene->selected != nullptr)
	{
		App->scene->selected->DrawProperties();
		const char* components[] = { "Transform", "Renderer", "Camera", "Light", "Script"};

		if (ImGui::Button("Add Component", ImVec2(ImGui::GetWindowWidth(), 25)))
			ImGui::OpenPopup("component_popup");
		ImGui::SameLine();
		if (ImGui::BeginPopup("component_popup"))
		{
			ImGui::Text("Components");
			ImGui::Separator();
			for (int i = 0; i < IM_ARRAYSIZE(components); i++)
				if (ImGui::Selectable(components[i]))
				{
					ComponentType type = (ComponentType)i;
					if (App->scene->selected->GetComponent(type) != nullptr && 
						(type == ComponentType::Renderer || type == ComponentType::Transform))
					{
						openPopup = true;
						popUpSentence = MAXCOMPONENTS;
					}
					else if (App->scene->selected == App->scene->root)
					{
						openPopup = true;
						popUpSentence = WORLDGO;
					}
					else
					{
						App->scene->selected->CreateComponent(type);
					}
				}
			ImGui::EndPopup();
		}

	}
	DrawWarningPopup();
	ImGui::PopID();
	ImGui::End();
}

void PanelInspector::SetFocus()
{
	focus = true;
}


void PanelInspector::DrawWarningPopup()
{
	if (openPopup && !ImGui::IsPopupOpen(WARNINGPOPUP))
	{
		ImGui::OpenPopup(WARNINGPOPUP);
		openPopup = false;
	}
	if (ImGui::BeginPopupModal(WARNINGPOPUP, NULL))
	{
		ImGui::Text(popUpSentence.c_str());
		ImGui::Separator();
		if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::SetItemDefaultFocus();
		ImGui::EndPopup();
	}
}