#include "PlayerStateFirstAttack.h"
#include "PlayerMovement.h"

#include "GameObject.h"
#include "ComponentTransform.h"

PlayerStateFirstAttack::PlayerStateFirstAttack(PlayerMovement * PM, const char * trigger,
	math::float3 boxSize, float minTime, float maxTime) :
	PlayerStateAttack(PM, trigger, boxSize, minTime, maxTime)
{
	this->minTime = 0.40f;
	this->maxTime = 0.65f;
}

PlayerStateFirstAttack::~PlayerStateFirstAttack()
{
}

void PlayerStateFirstAttack::Enter()
{
	player->gameobject->transform->LookAtMouse();

}

void PlayerStateFirstAttack::CheckInput()
{
	if (timer > duration * minTime)
	{
		if (player->IsAtacking())
		{
			player->currentState = (PlayerState*)player->secondAttack;
			return;
		}
	}
	
	if (timer > duration * maxTime) //CAN SWITCH?
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
