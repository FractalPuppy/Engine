#ifndef __PLAYERSTATESECONDATTACK_H_
#define __PLAYERSTATESECONDATTACK_H_

#include "PlayerStateAttack.h"

class PlayerStateSecondAttack :
	public PlayerStateAttack
{
public:
	PlayerStateSecondAttack(PlayerMovement * PM, const char * trigger, math::float3 boxSize, float minTime, float maxTime);
	~PlayerStateSecondAttack();

	void CheckInput() override;

};

#endif // __PLAYERSTATESECONDATTACK_H_