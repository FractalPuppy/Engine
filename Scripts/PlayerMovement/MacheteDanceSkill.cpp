#include "MacheteDanceSkill.h"

#include "Application.h"
#include "ModuleTime.h"
#include "ComponentTransform.h"
#include "GameObject.h"

#include "PlayerMovement.h"

MacheteDanceSkill::MacheteDanceSkill(PlayerMovement* PM, const char* trigger) : BasicSkill(PM, trigger)
{
}


MacheteDanceSkill::~MacheteDanceSkill()
{
}

void MacheteDanceSkill::Start()
{

	
}

void MacheteDanceSkill::Prepare()
{
	for (size_t i = 0; i < spinMachetes.size(); i++)
	{
		spinMachetes[i]->SetActive(true);
	}
	player->macheteDanceActivated = true;
}

void MacheteDanceSkill::Update()
{
	BasicSkill::Update();
}

void MacheteDanceSkill::RotateMachetes()
{
	float rotationAmount = player->App->time->gameDeltaTime * macheteRotationSpeed;
	spinMachetes[0]->parent->transform->Rotate(math::float3(0, -rotationAmount * 0.15f, 0));
	for (size_t i = 0; i < spinMachetes.size(); i++)
	{
		spinMachetes[i]->transform->Rotate(math::float3(0, rotationAmount, 0));
	}
}
