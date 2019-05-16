#include "PlayerStateIdle.h"



PlayerStateIdle::PlayerStateIdle(PlayerMovement* PM)
{
	player = PM;
	trigger = "Idle";
}


PlayerStateIdle::~PlayerStateIdle()
{
}

void PlayerStateIdle::Update()
{
}

void PlayerStateIdle::CheckInput()
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
