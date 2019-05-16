#ifndef __PLAYERSTATEFIRSTATTACK_H_
#define __PLAYERSTATEFIRSTATTACK_H_

#include "PlayerState.h"

class PlayerStateFirstAttack :
	public PlayerState
{
public:
	PlayerStateFirstAttack(PlayerMovement* PM);
	~PlayerStateFirstAttack();

	void Update() override;

	void CheckInput() override;

private:
	float duration = 1.5f;
};

#endif // __PLAYERSTATEFIRSTATTACK_H_