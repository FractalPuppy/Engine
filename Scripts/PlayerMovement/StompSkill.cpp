#include "StompSkill.h"

#include "PlayerMovement.h"

#include "Application.h"
#include "ModuleNavigation.h"
#include "ModuleTime.h"

#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentBoxTrigger.h"

#include "PlayerState.h"

StompSkill::StompSkill(PlayerMovement* PM, const char* trigger, ComponentBoxTrigger* attackBox) : MeleeSkill(PM, trigger, attackBox)
{
}


StompSkill::~StompSkill()
{
}

void StompSkill::Start()
{
	MeleeSkill::Start();

	player->gameobject->transform->LookAtMouse();

	if (StompFX != nullptr)
	{
		StompFX->SetActive(true);
	}
	player->ResetCooldown(HUD_BUTTON_1);

	//Create the hitbox
	boxSize = math::float3(200.f, 200.f, 200.f);

	// Set delay for hit
	hitDelay = 0.7f;
}

void StompSkill::UseSkill()
{
	if (player->attackBoxTrigger != nullptr && !player->attackBoxTrigger->enabled)
	{
		// Update hitbox
		math::float3 target;
		if (player->App->navigation->FindIntersectionPoint(player->gameobject->transform->GetPosition(), target))
		{
			float distance = target.Distance(player->gameobject->transform->GetPosition());

			if (distance < range)
			{
				player->attackBoxTrigger->SetBoxPosition(boxPosition.x, boxPosition.y, boxPosition.z + distance);
			}
			else
			{
				player->attackBoxTrigger->SetBoxPosition(boxPosition.x, boxPosition.y, boxPosition.z + range);
			}
		}
	}
}

void StompSkill::CheckInput()
{
	if (timer > player->currentState->duration) //CAN SWITCH?
	{
		if (player->IsUsingSkill())
		{
			player->currentState = (PlayerState*)player->attack;
		}
		else if (player->IsMovingToAttack())
		{
			player->currentState = (PlayerState*)player->walkToHit;
		}
		else if (player->IsMovingToItem())
		{
			player->currentState = (PlayerState*)player->walkToPickItem;
		}
		else if (player->IsMoving())
		{
			player->currentState = (PlayerState*)player->walk;
		}
	}
}
