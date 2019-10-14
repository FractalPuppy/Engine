#include "MacheteDanceSkill.h"

#include "Application.h"
#include "ModuleTime.h"
#include "ComponentTransform.h"
#include "ComponentRenderer.h"
#include "ComponentBoxTrigger.h"
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
		// Set dissolve at 1.0f
		ComponentRenderer* macheteDanceRenderer = spinMachetes[i]->GetComponent<ComponentRenderer>();
		macheteDanceRenderer->dissolveAmount = 1.0f;

		// Dissable hitboxes
		ComponentBoxTrigger* hitBox = spinMachetes[i]->GetComponent<ComponentBoxTrigger>();
		hitBox->Enable(false);

		spinMachetes[i]->SetActive(true);
	}
	player->macheteDanceActivated = true;
}

void MacheteDanceSkill::Update()
{
	for (size_t i = 0; i < spinMachetes.size(); i++)
	{
		// Dissolve animation
		ComponentRenderer* macheteDanceRenderer = spinMachetes[i]->GetComponent<ComponentRenderer>();
		macheteDanceRenderer->dissolveAmount = MAX(1.0f - (timer / duration), 0.0f);
	}
	BasicSkill::Update();
}

void MacheteDanceSkill::Exit()
{
	// Enable hitboxes
	for (size_t i = 0; i < spinMachetes.size(); i++)
	{
		ComponentBoxTrigger* hitBox = spinMachetes[i]->GetComponent<ComponentBoxTrigger>();
		hitBox->Enable(true);
	}
	BasicSkill::Exit();
}

void MacheteDanceSkill::RotateMachetes()
{
	float rotationAmount = player->App->time->gameDeltaTime * macheteRotationSpeed;
	spinMachetes[0]->parent->transform->Rotate(math::float3(0, -rotationAmount * 0.2f, 0));
	for (size_t i = 0; i < spinMachetes.size(); i++)
	{
		spinMachetes[i]->transform->Rotate(math::float3(0, rotationAmount, 0));
	}
}
