#include "PlayerState.h"
#include "PlayerMovement.h"

#include "Application.h"
#include "ModuleTime.h"

PlayerState::PlayerState(PlayerMovement * PM, const char * trigger, 
	math::float3 boxSize, float minTime, float maxTime) : 
	player(PM), trigger(trigger), boxSize(boxSize), minTime(minTime), maxTime(maxTime)
{
}

PlayerState::~PlayerState()
{
}

void PlayerState::UpdateTimer()
{
	timer += player->App->time->gameDeltaTime;
}
