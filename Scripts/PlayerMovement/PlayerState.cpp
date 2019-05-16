#include "PlayerState.h"

#include "Application.h"
#include "ModuleTime.h"

PlayerState::PlayerState()
{
}


PlayerState::~PlayerState()
{
}

void PlayerState::UpdateTimer()
{
	timer += player->Appl->time->gameDeltaTime;
}
