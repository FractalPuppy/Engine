#include "PlayerStateDeath.h"
#include "PlayerMovement.h"

PlayerStateDeath::PlayerStateDeath(PlayerMovement * PM, const char * trigger) :
	PlayerState(PM, trigger)
{
}

PlayerStateDeath::~PlayerStateDeath()
{
}
