#include "PlayerStateUppercut.h"

#include "PlayerMovement.h"
#include "Application.h"
#include "ModuleScene.h"

#include "GameObject.h"

#include "ComponentTransform.h"

PlayerStateUppercut::PlayerStateUppercut(PlayerMovement* PM)
{
	player = PM;
	trigger = "Uppercut";
}


PlayerStateUppercut::~PlayerStateUppercut()
{
}

void PlayerStateUppercut::Update()
{
	/*player->pathIndex = 0;
	player->path.clear();
	math::float3 attackPosition;
	if (player->Appl->scene->Intersects(attackPosition, "floor"))
	{
		player->gameobject->transform->LookAt(attackPosition);
	}*/
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
