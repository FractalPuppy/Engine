#include "PlayerStateFirstAttack.h"

#include "PlayerMovement.h"
#include "Application.h"
#include "ModuleScene.h"

#include "GameObject.h"

#include "ComponentTransform.h"

PlayerStateFirstAttack::PlayerStateFirstAttack(PlayerMovement* PM)
{
	player = PM;
	trigger = "FirstAttack";
}

PlayerStateFirstAttack::~PlayerStateFirstAttack()
{
}

void PlayerStateFirstAttack::Update()
{
	//player->pathIndex = 0;
	//player->path.clear();
	//math::float3 attackPosition;
	//if (player->Appl->scene->Intersects(attackPosition, "floor"))
	//{
	//	player->gameobject->transform->LookAt(attackPosition);
	//}
}

void PlayerStateFirstAttack::CheckInput()
{
	if (timer > duration * 0.8)
	{
		if (player->IsAtacking())
		{
			player->currentState = (PlayerState*)player->secondAttack;
		}
	}
	if (timer > duration * 1.5) //CAN SWITCH?
	{
		
		if (player->IsUsingFirstSkill())
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
