#include "PlayerStateBombDrop.h"
#include "PlayerMovement.h"

#include "GameObject.h"
#include "Application.h"

#include "ModuleNavigation.h"
#include "ModuleTime.h"

#include "ComponentTransform.h"
#include "ComponentBoxTrigger.h"


PlayerStateBombDrop::PlayerStateBombDrop(PlayerMovement* PM, const char* trigger, math::float3 boxSize) :
	PlayerState(PM, trigger, boxSize)
{
}


PlayerStateBombDrop::~PlayerStateBombDrop()
{
	path.clear();
}

void PlayerStateBombDrop::Update()
{
	if (path.size() > 0 && timer > bombDropPreparationTime)
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
			player->gameobject->transform->SetPosition(currentPosition + bombDropSpeed * direction * player->App->time->gameDeltaTime);
		}
	}

	if (player->attackBoxTrigger != nullptr && !hitboxCreated && timer > duration * minTime && timer < duration * maxTime)
	{
		//Create the hitbox
		player->attackBoxTrigger->Enable(true);
		player->attackBoxTrigger->SetBoxSize(boxSize);
		boxPosition = player->transform->up * 100.f; //this front stuff isnt working well when rotating the chicken
		player->attackBoxTrigger->SetBoxPosition(boxPosition.x, boxPosition.y, boxPosition.z + 100.f);
		hitboxCreated = true;
	}
	if (player->attackBoxTrigger != nullptr && hitboxCreated && timer > duration * maxTime)
	{
		player->attackBoxTrigger->Enable(false);
		hitboxCreated = false;
	}
}
void PlayerStateBombDrop::Enter()
{
	if (player->App->navigation->NavigateTowardsCursor(player->gameobject->transform->position, path,
		math::float3(player->OutOfMeshCorrectionXZ, player->OutOfMeshCorrectionY, player->OutOfMeshCorrectionXZ), intersectionPoint))
	{
		pathIndex = 0;
		player->gameobject->transform->LookAt(intersectionPoint);
		if (bombDropFX)
		{
			bombDropFX->SetActive(true);
		}
		player->UseSkill(SkillType::DASH);
	}
}
void PlayerStateBombDrop::CheckInput()
{
	if (timer > duration) // can switch?¿¿?
	{
		if (bombDropFX)
		{
			bombDropFX->SetActive(false);
		}
		/*if (dashMesh)
		{
			dashMesh->SetActive(false);
		}*/

		if (player->IsAtacking())
		{
			player->currentState = (PlayerState*)player->firstAttack;
			return;
		}
		if (player->IsUsingFirstSkill()) //cooldown?
		{
			player->currentState = player->allSkills[player->activeSkills[0]]->state;
			return;
		}
		if (player->IsUsingSecondSkill())
		{
			player->currentState = player->allSkills[player->activeSkills[1]]->state;
			return;
		}
		if (player->IsMoving())
		{
			player->currentState = (PlayerState*)player->walk;
			return;
		}
		player->currentState = (PlayerState*)player->idle;
	}
}