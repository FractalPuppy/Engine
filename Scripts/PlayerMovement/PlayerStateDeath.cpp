#include "PlayerStateDeath.h"



PlayerStateDeath::PlayerStateDeath(PlayerMovement* PM)
{
	player = PM;
	trigger = "Death";
}


PlayerStateDeath::~PlayerStateDeath()
{
}

void PlayerStateDeath::Update()
{
}

void PlayerStateDeath::CheckInput()
{
}
