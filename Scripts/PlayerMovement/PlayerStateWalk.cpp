#include "PlayerMovement.h"

#include "Application.h"

#include "ModuleScene.h"
#include "ModuleNavigation.h"
#include "ModuleTime.h"

#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentAnimation.h"

#include "PlayerStateWalk.h"
#include "PlayerStateIdle.h"

#include "JSON.h"
#include <assert.h>
#include <string>
#include "imgui.h"
#include "Globals.h"
#include "debugdraw.h"

#define CLOSE_ENOUGH 400.0f

PlayerStateWalk::PlayerStateWalk(PlayerMovement* PM)
{
	player = PM;
	trigger = "Walk";
}


PlayerStateWalk::~PlayerStateWalk()
{
}

void PlayerStateWalk::Update()
{
	/*math::float3 intersectionPoint = math::float3::inf;
	if (player->Appl->scene->Intersects(intersectionPoint, "floor"))
	{
		player->Appl->navigation->FindPath(player->gameobject->transform->position, intersectionPoint, player->path);
		player->pathIndex = 0;
	}
	if (player->path.size() > 0)
	{
		math::float3 currentPosition = player->gameobject->transform->GetPosition();
		while (player->pathIndex < player->path.size() && currentPosition.DistanceSq(player->path[player->pathIndex]) < CLOSE_ENOUGH)
		{
			player->pathIndex++;
		}
		if (player->pathIndex < player->path.size())
		{
			player->gameobject->transform->LookAt(player->path[player->pathIndex]);
			math::float3 direction = (player->path[player->pathIndex] - currentPosition).Normalized();
			player->gameobject->transform->SetPosition(currentPosition + player->speed*direction*player->Appl->time->gameDeltaTime);
		}
	}	*/
}

void PlayerStateWalk::CheckInput()
{
	if (timer > duration)
	{
		if (player->IsAtacking())
		{
			player->currentState = (PlayerState*)player->firstAttack;
		}
		else if (player->IsUsingFirstSkill())
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
