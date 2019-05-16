#ifndef __PLAYERSTATEDASH_H_
#define __PLAYERSTATEDASH_H_

#include "PlayerState.h"

class PlayerStateDash :
	public PlayerState
{
public:
	PlayerStateDash(PlayerMovement* PM);
	~PlayerStateDash();

	void Update() override;

	void CheckInput() override;

private:
	float duration = 0.8f;
};

#endif // __PLAYERSTATEDASH_H_