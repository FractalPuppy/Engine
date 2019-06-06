#ifndef __PLAYERSTATEWALK_H_
#define __PLAYERSTATEWALK_H_

#include "PlayerState.h"

class PlayerStateWalk :	public PlayerState
{
public:
	PlayerStateWalk(PlayerMovement * PM, const char * trigger);
	~PlayerStateWalk();

	void Update() override;
	void CheckInput() override;
	void Enter() override;

public:
	float duration = 1.5f;
	std::vector<float3>path;
	unsigned pathIndex = 0;
	GameObject* dustParticles = nullptr;

private:
	float moveTimer = 0.0f;
};

#endif // __PLAYERSTATEWALK_H_