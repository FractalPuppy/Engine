#include "PlayerStateIdle.h"
#include "PlayerMovement.h"

PlayerStateIdle::PlayerStateIdle(PlayerMovement * PM, const char * trigger):
	PlayerState(PM, trigger)
{
}

PlayerStateIdle::~PlayerStateIdle()
{
}

void PlayerStateIdle::Update()
{
}

void PlayerStateIdle::CheckInput()
{
	if (player->IsUsingSkill() || player->IsAttacking())
	{
		player->currentState = (PlayerState*)player->attack;
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
