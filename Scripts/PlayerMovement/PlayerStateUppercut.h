#ifndef __PLAYERSTATEUPPERCUT_H_
#define __PLAYERSTATEUPPERCUT_H_

#include "PlayerState.h"

class PlayerStateUppercut :
	public PlayerState
{
public:
	PlayerStateUppercut(PlayerMovement* PM);
	~PlayerStateUppercut();

	void Update() override;

	void CheckInput() override;

private:
	float duration = 1.5f;
};

#endif // __PLAYERSTATEUPPERCUT_H_