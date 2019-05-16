#ifndef __PLAYERSTATESECONDATTACK_H_
#define __PLAYERSTATESECONDATTACK_H_

#include "PlayerState.h"
class PlayerStateSecondAttack :
	public PlayerState
{
public:
	PlayerStateSecondAttack(PlayerMovement* PM);
	~PlayerStateSecondAttack();

	void Update() override;

	void CheckInput() override;

private:
	float duration = 1.5f;
};

#endif // __PLAYERSTATESECONDATTACK_H_