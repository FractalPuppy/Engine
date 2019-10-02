#include "GodMode.h"

#include "Application.h"
#include "ModuleInput.h"
#include "ModuleRender.h"
#include "ModuleScene.h"
#include "GameObject.h"
#include "PlayerMovement/PlayerMovement.h"
#include "ExperienceController/ExperienceController.h"
#include "GameLoop/GameLoop.h"
#include "imgui.h"

#define EXPOSURE_OFFSET 0.5f
#define EXPOSURE_MIN 0.5f
#define EXPOSURE_MAX 20.0f

GodMode_API Script* CreateScript()
{
	GodMode* instance = new GodMode;
	return instance;
}

void GodMode::Start()
{
	playerGO = App->scene->FindGameObjectByName("Player");
	gameLoopGO = App->scene->FindGameObjectByName("GameController");
	xpGO = App->scene->FindGameObjectByName("Xp");
}

void GodMode::Update()
{
	SwitchLevel();
	AddExperience();
	InfiniteHealth();
	InfiniteMana();
	MaxStats();
	Exposure();
}

void GodMode::Expose(ImGuiContext* context)
{
}

void GodMode::SwitchLevel() const
{
	unsigned level = 1u;
	if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
	{
		level = 1u;
	}
	else if (App->input->GetKey(SDL_SCANCODE_F2) == KEY_DOWN)
	{
		level = 2u;
	}
	else
	{
		return;
	}
	gameLoopGO->GetComponent<GameLoop>()->LoadLvl(level);
}

void GodMode::AddExperience() const
{
	if (App->input->GetKey(SDL_SCANCODE_F3) != KEY_DOWN) return;
	xpGO->GetComponent<ExperienceController>()->AddXP(100.f);
}

void GodMode::InfiniteHealth() const
{
	if (App->input->GetKey(SDL_SCANCODE_F4) != KEY_DOWN) return;
	if (playerGO != nullptr)
	{
		playerGO->GetComponent<PlayerMovement>()->ToggleInfiniteHealth();
	}
}

void GodMode::InfiniteMana() const
{
	if (App->input->GetKey(SDL_SCANCODE_F5) != KEY_DOWN) return;
	if (playerGO != nullptr)
	{
		playerGO->GetComponent<PlayerMovement>()->ToggleInfiniteMana();
	}
}

void GodMode::MaxStats() const
{
	if (App->input->GetKey(SDL_SCANCODE_F6) != KEY_DOWN) return;
	if (playerGO != nullptr)
	{
		playerGO->GetComponent<PlayerMovement>()->ToggleMaxStats();
	}
}

void GodMode::Exposure() const
{
	if (App->input->GetKey(SDL_SCANCODE_F7) == KEY_DOWN)
	{
		App->renderer->exposure = MIN(EXPOSURE_MAX, App->renderer->exposure + EXPOSURE_OFFSET);
	}
	else if (App->input->GetKey(SDL_SCANCODE_F8) == KEY_DOWN)
	{
		App->renderer->exposure = MAX(EXPOSURE_MIN, App->renderer->exposure - EXPOSURE_OFFSET);
	}
}