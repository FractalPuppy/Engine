#include "PlayerStateUppercut.h"

#include "PlayerMovement.h"
#include "Application.h"
#include "ModuleScene.h"

#include "GameObject.h"

#include "ComponentTransform.h"
#include "ComponentBoxTrigger.h"

PlayerStateUppercut::PlayerStateUppercut(PlayerMovement * PM, const char * trigger,
	math::float3 boxSize) : PlayerState(PM, trigger, boxSize)
{
}

PlayerStateUppercut::~PlayerStateUppercut()
{
}

void PlayerStateUppercut::Update()
{
	if (player->attackBoxTrigger != nullptr && !hitboxCreated && timer > duration * minTime && timer < duration * maxTime)
	{
		//Create the hitbox
		player->attackBoxTrigger->Enable(true);
		player->attackBoxTrigger->SetBoxSize(boxSize);
		boxPosition = player->transform->up *100.f; //this front stuff isnt working well when rotating the chicken
		player->attackBoxTrigger->SetBoxPosition(boxPosition.x, boxPosition.y, boxPosition.z + 100.f);
		hitboxCreated = true;
	}
	if (player->attackBoxTrigger != nullptr && hitboxCreated && timer > duration * maxTime)
	{
		player->attackBoxTrigger->Enable(false);
		hitboxCreated = false;
	}
}

void PlayerStateUppercut::Enter()
{

}

void PlayerStateUppercut::Exit()
{

}

void PlayerStateUppercut::CheckInput()
{
	if (timer > duration) // can switch??¿?¿?
	{
		if (player->IsAtacking())
		{
			player->currentState = (PlayerState*)player->firstAttack;
		}
		else if (player->IsUsingFirstSkill())
		{
			player->currentState = (PlayerState*)player->dash;
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
