#ifndef __PLAYERSTATETHIRDATTACK_H_
#define __PLAYERSTATETHIRDATTACK_H_

#include "PlayerState.h"
class PlayerStateThirdAttack :
	public PlayerState
{
public:
	PlayerStateThirdAttack(PlayerMovement* PM);
	~PlayerStateThirdAttack();

	void Update() override;

	void CheckInput() override;

private:
	float duration = 1.5f;
};

#endif // __PLAYERSTATETHIRDATTACK_H_