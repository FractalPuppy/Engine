#include "Application.h"

#include "ModuleScene.h"
#include "ModuleEditor.h"
#include "ModuleFileSystem.h"
#include "ModuleScript.h"

#include "GameObject.h"
#include "Component.h"
#include "BaseScript.h"

#include "PanelInspector.h"
#include "PanelHierarchy.h"
#include "imgui.h"

#define WARNINGPOPUP "WarningPopup"
#define MAXCOMPONENTS "This GameObject cannot have this component twice! \n\n"
#define WORLDGO "Cannot add components to World gameobject"

PanelInspector::PanelInspector()
{
	componentList = { {"Transform", ComponentType::Transform},{"Transform2D", ComponentType::Transform2D}, {"Renderer", ComponentType::Renderer},
	{"Camera", ComponentType::Camera},  {"Light", ComponentType::Light},
	{"Animation", ComponentType::Animation}, {"Particle System", ComponentType::Particles},
	{"Trail Renderer", ComponentType::Trail},
	{"Image", ComponentType::Image}, {"Reverb Zone", ComponentType::ReverbZone}, {"Audio Listener", ComponentType::AudioListener},
	{"Audio Source", ComponentType::AudioSource}, {"Box Trigger", ComponentType::BoxTrigger},
	{"Volumetric light", ComponentType::VolumetricLight} };
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
	if (App->scene->selected != nullptr && App->scene->root != nullptr && App->scene->selected != App->scene->root)
	{
		App->scene->selected->DrawProperties();

		if (ImGui::Button("Add Component", ImVec2(ImGui::GetWindowWidth(), 25)))
			ImGui::OpenPopup("component_popup");
		if (ImGui::Button("Add Script", ImVec2(ImGui::GetWindowWidth(), 25)))
		{
			ImGui::OpenPopup("script_popup");
			scriptList = App->fsystem->GetFolderContent(SCRIPTS, false);
		}
		ImGui::SameLine();
		if (ImGui::BeginPopup("component_popup"))
		{
			ImGui::Text("Components");
			ImGui::Separator();
			for (int i = 0; i < componentList.size(); i++)
				if (ImGui::Selectable(componentList[i].first))
				{
					ComponentType type = componentList[i].second;
					if (App->scene->selected->GetComponentOld(type) != nullptr && 
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
						App->scene->TakePhoto();
						App->scene->selected->CreateComponent(type);
					}
				}
			ImGui::EndPopup();
		}
		if (ImGui::BeginPopup("script_popup"))
		{
			ImGui::Text("Scripts");
			ImGui::Separator();
			for (int i = 0; i < scriptList.size(); i++)
				if (ImGui::Selectable(scriptList[i].c_str()))
				{
					Script* script = App->scripting->GetScript(scriptList[i]);
					script->SetGameObject(App->scene->selected);
					App->scene->selected->components.push_back((Component*)script);
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