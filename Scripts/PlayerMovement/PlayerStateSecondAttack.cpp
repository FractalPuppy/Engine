#include "PlayerStateSecondAttack.h"

#include "PlayerMovement.h"
#include "Application.h"
#include "ModuleScene.h"

PlayerStateSecondAttack::PlayerStateSecondAttack(PlayerMovement* PM, const char* trigger, math::float3 boxSize,
	float minTime, float maxTime): PlayerStateAttack(PM, trigger, boxSize, minTime, maxTime)
{
	minTime = 0.85f;
	maxTime = 0.9f;
}

PlayerStateSecondAttack::~PlayerStateSecondAttack()
{
}

void PlayerStateSecondAttack::CheckInput()
{

	if (timer > duration * minTime)
	{
		if (player->IsAtacking())
		{
			player->currentState = (PlayerState*)player->thirdAttack;
		}
	}
	if (timer > duration * maxTime) // can switch??¿?¿?
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
