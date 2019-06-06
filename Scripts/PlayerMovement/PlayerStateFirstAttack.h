#ifndef __PLAYERSTATEFIRSTATTACK_H_
#define __PLAYERSTATEFIRSTATTACK_H_

#include "PlayerStateAttack.h"

class PlayerStateFirstAttack :
	public PlayerStateAttack
{
public:
	PlayerStateFirstAttack(PlayerMovement * PM, const char* trigger,
		math::float3 boxSize, float minTime, float maxTime);
	~PlayerStateFirstAttack();

	void Enter() override;
	void CheckInput() override;

};

#endif // __PLAYERSTATEFIRSTATTACK_H_