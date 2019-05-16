#ifndef __PLAYERSTATEDEATH_H_
#define __PLAYERSTATEDEATH_H_

#include "PlayerMovement.h"
#include "PlayerState.h"

class PlayerStateDeath :
	public PlayerState
{
public:
	PlayerStateDeath(PlayerMovement* PM);
	~PlayerStateDeath();

	void Update() override;

	void CheckInput() override;

	float duration = 1.5f;
};

#endif // __PLAYERSTATEDEATH_H_