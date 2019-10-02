#include "PlayerStateDeath.h"
#include "PlayerMovement.h"
#include "Application.h"
#include "ModuleScene.h"

PlayerStateDeath::PlayerStateDeath(PlayerMovement * PM, const char * trigger) :
	PlayerState(PM, trigger)
{
}

void PlayerStateDeath::Enter()
{
	player->App->scene->enemyHovered.object = nullptr;
}

PlayerStateDeath::~PlayerStateDeath()
{
}
