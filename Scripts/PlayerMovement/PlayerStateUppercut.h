#ifndef __PLAYERSTATEUPPERCUT_H_
#define __PLAYERSTATEUPPERCUT_H_

#include "PlayerState.h"

class PlayerStateUppercut :
	public PlayerState
{
public:
	PlayerStateUppercut(PlayerMovement * PM, const char * trigger, math::float3 boxSize);
	~PlayerStateUppercut();

	void Update() override;
	void Enter() override;
	void Exit() override;
	void CheckInput() override;
	
};

#endif // __PLAYERSTATEUPPERCUT_H_