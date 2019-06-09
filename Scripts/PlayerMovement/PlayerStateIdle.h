#ifndef __PLAYERSTATEIDLE_H_
#define __PLAYERSTATEIDLE_H_

#include "PlayerState.h"

class PlayerStateIdle :
	public PlayerState
{
public:
	PlayerStateIdle(PlayerMovement* PM, const char* trigger);
	~PlayerStateIdle();

	void Update() override;

	void CheckInput() override;
};

#endif // __PLAYERSTATEIDLE_H_