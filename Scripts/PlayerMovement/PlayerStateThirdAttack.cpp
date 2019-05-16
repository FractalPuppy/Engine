#include "PlayerStateThirdAttack.h"

#include "PlayerMovement.h"
#include "Application.h"
#include "ModuleScene.h"

#include "GameObject.h"

#include "ComponentTransform.h"

PlayerStateThirdAttack::PlayerStateThirdAttack(PlayerMovement* PM)
{
	player = PM;
	trigger = "ThirdAttack";
}


PlayerStateThirdAttack::~PlayerStateThirdAttack()
{
}

void PlayerStateThirdAttack::Update()
{
	//player->pathIndex = 0;
	//player->path.clear();
	//math::float3 attackPosition;
	//if (player->Appl->scene->Intersects(attackPosition, "floor"))
	//{
	//	player->gameobject->transform->LookAt(attackPosition);
	//}
}

void PlayerStateThirdAttack::CheckInput()
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
