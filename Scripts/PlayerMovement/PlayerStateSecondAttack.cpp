#include "PlayerStateSecondAttack.h"

#include "PlayerMovement.h"
#include "Application.h"
#include "ModuleScene.h"

#include "GameObject.h"

#include "ComponentTransform.h"

PlayerStateSecondAttack::PlayerStateSecondAttack(PlayerMovement* PM)
{
	player = PM;
	trigger = "SecondAttack";
}


PlayerStateSecondAttack::~PlayerStateSecondAttack()
{
}

void PlayerStateSecondAttack::Update()
{
	/*player->pathIndex = 0;
	player->path.clear();
	math::float3 attackPosition;
	if (player->Appl->scene->Intersects(attackPosition, "floor"))
	{
		player->gameobject->transform->LookAt(attackPosition);
	}*/
}

void PlayerStateSecondAttack::CheckInput()
{

	if (timer > duration * 0.5)
	{
		if (player->IsAtacking())
		{
			player->currentState = (PlayerState*)player->thirdAttack;
		}
	}
	if (timer > duration * 1.5) // can switch??¿?¿?
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
