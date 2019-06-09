#include "PlayerStateAttack.h"
#include "PlayerMovement.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentBoxTrigger.h"

PlayerStateAttack::PlayerStateAttack(PlayerMovement * PM, const char * trigger, math::float3 boxSize, float minTime, float maxTime):
	PlayerState(PM, trigger, boxSize, minTime, maxTime)
{
}

PlayerStateAttack::~PlayerStateAttack()
{
}

void PlayerStateAttack::Update()
{
	if (player->attackBoxTrigger != nullptr && !hitboxCreated && timer > duration * minTime && timer < duration * maxTime)
	{
		//Create the hitbox
		player->attackBoxTrigger->Enable(true);
		player->attackBoxTrigger->SetBoxSize(boxSize);
		boxPosition = player->transform->up * 100.f; //this front stuff isnt working well when rotating the chicken
		player->attackBoxTrigger->SetBoxPosition(boxPosition.x, boxPosition.y, boxPosition.z + 100.f);
		hitboxCreated = true;
	}
	if (player->attackBoxTrigger != nullptr && hitboxCreated && timer > duration * maxTime)
	{
		player->attackBoxTrigger->Enable(false);
		hitboxCreated = false;
	}
}
