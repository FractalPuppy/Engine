#ifndef __PLAYERSTATEWALK_H_
#define __PLAYERSTATEWALK_H_

#include "BaseScript.h"
#include "PlayerState.h"

class PlayerStateWalk :	public PlayerState
{
public:
	PlayerStateWalk(PlayerMovement* PM);
	~PlayerStateWalk();

	void Update() override;
	void CheckInput() override;
	float duration = 1.5f;
};

#endif // __PLAYERSTATEWALK_H_