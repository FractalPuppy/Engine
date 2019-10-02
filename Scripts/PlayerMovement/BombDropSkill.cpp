#include "BombDropSkill.h"

#include "PlayerMovement.h"

#include "Application.h"
#include "ModuleNavigation.h"
#include "ModuleTime.h"

#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentBoxTrigger.h"

#include "PlayerState.h"

BombDropSkill::BombDropSkill(PlayerMovement* PM, const char* trigger, ComponentBoxTrigger* attackBox) :
	MeleeSkill(PM, trigger, attackBox)
{
}


BombDropSkill::~BombDropSkill()
{
	path.clear();
}

void BombDropSkill::Start()
{
	MeleeSkill::Start();
	player->App->navigation->setPlayerBB(player->gameobject->bbox);
	if (player->App->navigation->NavigateTowardsCursor(player->gameobject->transform->position, path,
		math::float3(player->OutOfMeshCorrectionXZ, player->OutOfMeshCorrectionY, player->OutOfMeshCorrectionXZ),
		intersectionPoint, bombDropMaxDistance, PathFindType::NODODGE))
	{
		pathIndex = 0;
		player->gameobject->transform->LookAt(intersectionPoint);
		if (bombDropFX)
		{
			bombDropFX->SetActive(true);
		}
		player->ResetCooldown(HUD_BUTTON_E);
	}

	//Create the hitbox
	boxSize = math::float3(250.f, 100.f, 250.f);
	//attackBoxTrigger.set
	// Set delay for hit
	hitDelay = 0.5f;
}

void BombDropSkill::UseSkill()
{
	if (path.size() > 0 && timer > bombDropPreparationTime && timer < hitDelay)
	{
		math::float3 currentPosition = player->gameobject->transform->GetPosition();
		while (pathIndex < path.size() && currentPosition.DistanceSq(path[pathIndex]) < MINIMUM_PATH_DISTANCE)
		{
			pathIndex++;
		}
		if (pathIndex < path.size())
		{
			player->gameobject->transform->LookAt(path[pathIndex]);
			math::float3 direction = (path[pathIndex] - currentPosition).Normalized();
			player->gameobject->transform->SetPosition(currentPosition + bombDropSpeed * direction * player->App->time->fullGameDeltaTime);
		}
	}

	if (player->attackBoxTrigger != nullptr && !player->attackBoxTrigger->enabled)
	{
		// Update hitbox
		boxPosition = player->transform->up * 100.f; //this front stuff isnt working well when rotating the chicken
		player->attackBoxTrigger->SetBoxPosition(boxPosition.x, boxPosition.y, boxPosition.z + 200.f);
	}
}


void BombDropSkill::CheckInput()
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


