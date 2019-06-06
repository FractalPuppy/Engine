#ifndef __PLAYERSTATETHIRDATTACK_H_
#define __PLAYERSTATETHIRDATTACK_H_

#include "PlayerStateAttack.h"

class PlayerStateThirdAttack :
	public PlayerStateAttack
{
public:
	PlayerStateThirdAttack(PlayerMovement* PM, const char* trigger, math::float3 boxSize,
		float minTime, float maxTime);
	~PlayerStateThirdAttack();

	void CheckInput() override;

};

#endif // __PLAYERSTATETHIRDATTACK_H_