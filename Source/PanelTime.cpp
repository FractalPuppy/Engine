#include "Application.h"

#include "ModuleWindow.h"
#include "ModuleTime.h"
#include "ModuleScene.h"
#include "ModuleFileSystem.h"

#include "PanelTime.h"
#include "GameObject.h"

#include "imgui.h"

PanelTime::PanelTime()
{
}

PanelTime::~PanelTime()
{
}

void PanelTime::Draw()
{
	ImGui::Begin("Time", &enabled, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse);

	ImVec2 size = ImGui::GetWindowSize();
	ImGui::SetCursorPos(ImVec2(-(App->window->width - size.x) * 0.5f, -(App->window->height - size.y) * 0.5f));

	ImGui::BeginMenuBar();

	if (App->time->gameState == GameState::STOP) 
	{
		ImGui::PushStyleColor(ImGuiCol_Button, { 0.5f, 0.5f, 0.5f, 0.7f });
	}
	else 
	{
		ImGui::PushStyleColor(ImGuiCol_Button, { 0.3f, 0.5f, 0.3f, 0.7f });
	}

	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 1.0f, 1.0f, 1.0f, 0.2f });

	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 85) / 2);
	if (ImGui::ArrowButton("Play", ImGuiDir_Right)) 
	{
		if (App->time->gameState == GameState::STOP) 
		{
			App->time->StartGameClock();
			App->scene->SaveTemporaryScene();
			App->scene->root->OnPlay();
		}
		else 
		{
			App->time->StopGameClock();
			App->scene->LoadTemporaryScene();
		}
	}

	ImGui::PopStyleColor(2);
	if (App->time->gameState == GameState::PAUSE) 
	{
		ImGui::PushStyleColor(ImGuiCol_Button, { 0.3f, 0.5f, 0.3f, 0.7f });
	}
	else 
	{
		ImGui::PushStyleColor(ImGuiCol_Button, { 0.5f, 0.5f, 0.5f, 0.7f });
	}

	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 1.0f, 1.0f, 1.0f, 0.2f });
	ImGui::SameLine();

	if (ImGui::Button("||", { 23,23 })) 
	{
		if (App->time->gameState == GameState::RUN) 
		{
			App->time->PauseGameClock(true);
		}
		else if (App->time->gameState == GameState::PAUSE) 
		{
			App->time->PauseGameClock(false);
		}
	}

	ImGui::PopStyleColor(2);
	ImGui::SameLine();

	if (App->time->gameState == GameState::PAUSE) 
	{
		ImGui::PushStyleColor(ImGuiCol_Button, { 0.3f, 0.5f, 0.3f, 0.7f });
	}
	else 
	{
		ImGui::PushStyleColor(ImGuiCol_Button, { 0.5f, 0.5f, 0.5f, 0.7f });
	}

	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 1.0f, 1.0f, 1.0f, 0.2f });
	if (App->time->nextFrame) 
	{
		App->time->nextFrame = false;
		App->time->PauseGameClock(true);
	}

	if (ImGui::Button("->", { 23,23 }) && App->time->gameState == GameState::PAUSE) 
	{
		App->time->nextFrame = true;
		App->time->PauseGameClock(false);
	}

	ImGui::PopStyleColor(2);
	ImGui::EndMenuBar();

	ImGui::End();
}
