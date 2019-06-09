#include "PlayerStateDash.h"
#include "PlayerMovement.h"

#include "Application.h"

#include "ModuleScene.h"
#include "ModuleNavigation.h"
#include "ModuleTime.h"

#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentBoxTrigger.h"


#include "PlayerStateWalk.h"
#include "PlayerStateIdle.h"

#include "JSON.h"
#include <assert.h>
#include <string>
#include "imgui.h"
#include "Globals.h"
#include "debugdraw.h"

PlayerStateDash::PlayerStateDash(PlayerMovement * PM, const char * trigger, math::float3 boxSize) :
	PlayerState(PM, trigger, boxSize)
{
}

PlayerStateDash::~PlayerStateDash()
{
}

void PlayerStateDash::Update()
{

	if (path.size() > 0 && timer > dashPreparationTime)
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
			player->gameobject->transform->SetPosition(currentPosition + dashSpeed * direction * player->App->time->gameDeltaTime);
			if (dashMesh)
			{			
				dashMesh->transform->Scale(scalator);
				scalator -= scalatorDecay;
				scalator = MAX(1.0f, scalator);
			}
		}
	}

	if (player->attackBoxTrigger != nullptr && !hitboxCreated && timer > duration * minTime && timer < duration * maxTime)
	{
		//Create the hitbox
		player->attackBoxTrigger->Enable(true);
		player->attackBoxTrigger->SetBoxSize(boxSize);
		boxPosition = player->transform->up *100.f; //this front stuff isnt working well when rotating the chicken
		player->attackBoxTrigger->SetBoxPosition(boxPosition.x, boxPosition.y, boxPosition.z + 100.f);
		hitboxCreated = true;
	}
	if (player->attackBoxTrigger != nullptr &&hitboxCreated && timer > duration * maxTime)
	{
		player->attackBoxTrigger->Enable(false);
		hitboxCreated = false;
	}
}

void PlayerStateDash::Enter()
{
	if (player->App->navigation->NavigateTowardsCursor(player->gameobject->transform->position, path,
		math::float3(player->OutOfMeshCorrectionXZ, player->OutOfMeshCorrectionY, player->OutOfMeshCorrectionXZ), intersectionPoint))
	{
		pathIndex = 0;
		player->gameobject->transform->LookAt(intersectionPoint);
		if (dashFX)
		{
			dashFX->SetActive(true);
		}
		if (dashMesh)
		{
			dashMesh->SetActive(true);
			dashMesh->transform->scale = meshOriginalScale;			
			dashMesh->transform->Scale(1.0f);
			scalator = originalScalator;
		}
	}
}

void PlayerStateDash::CheckInput()
{
	if (timer > duration) // can switch?¿¿?
	{
		if (dashFX)
		{
			dashFX->SetActive(false);
		}
		if (dashMesh)
		{
			dashMesh->SetActive(false);
		}

		if (player->IsAtacking())
		{
			player->currentState = (PlayerState*)player->firstAttack;
		}
		else if (player->IsUsingFirstSkill()) //cooldown?
		{
			player->currentState = (PlayerState*)player->dash;
		}
		else if (player->IsUsingSecondSkill())
		{
			player->currentState = (PlayerState*)player->uppercut;
		}
		else if (player->IsMoving())
		{
			player->currentState = (PlayerState*)player->walk;
		}
		else
		{
			player->currentState = (PlayerState*)player->idle;
		}
	}
}
