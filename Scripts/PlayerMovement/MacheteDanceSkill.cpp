#include "MacheteDanceSkill.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleTime.h"
#include "ComponentTransform.h"
#include "GameObject.h"

#include "PlayerMovement.h"

#define MACHETE_SPIN "SpinMachete"

MacheteDanceSkill::MacheteDanceSkill(PlayerMovement* PM, const char* trigger) : BasicSkill(PM, trigger)
{
}


MacheteDanceSkill::~MacheteDanceSkill()
{
}

void MacheteDanceSkill::Start()
{
	// Spawn prefab
	GameObject* spawnedMachete = player->App->scene->Spawn("MacheteDance", player->gameobject);
	spinMachetes = player->App->scene->FindGameObjectsByTag(MACHETE_SPIN, spawnedMachete);
}

void MacheteDanceSkill::Prepare()
{
	for (size_t i = 0; i < spinMachetes.size(); i++)
	{
		spinMachetes[i]->SetActive(true);
	}
}

void MacheteDanceSkill::Update()
{
	BasicSkill::Update();
	RotateMachetes();
}

void MacheteDanceSkill::RotateMachetes()
{
	float rotationAmount = player->App->time->gameDeltaTime * boneRotationSpeed;
	for (size_t i = 0; i < spinMachetes.size(); i++)
	{
		spinMachetes[i]->transform->Rotate(math::float3(0, rotationAmount, 0));
	}
}
