#ifndef __PLAYERSTATEDEATH_H_
#define __PLAYERSTATEDEATH_H_

#include "PlayerState.h"

class PlayerMovement;

class PlayerStateDeath :
	public PlayerState
{
public:
	PlayerStateDeath(PlayerMovement* PM, const char* trigger);
	~PlayerStateDeath();
};

#endif // __PLAYERSTATEDEATH_H_