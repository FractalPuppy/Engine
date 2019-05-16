#ifndef __PLAYERSTATEIDLE_H_
#define __PLAYERSTATEIDLE_H_

#include "PlayerState.h"

class PlayerStateIdle :
	public PlayerState
{
public:
	PlayerStateIdle(PlayerMovement* PM);
	~PlayerStateIdle();

	void Update() override;

	void CheckInput() override;

	float duration = 1.5f;
};

#endif // __PLAYERSTATEIDLE_H_