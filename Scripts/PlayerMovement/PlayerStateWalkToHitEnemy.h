#ifndef __PLAYERSTATEWALKTOHIT_H_
#define __PLAYERSTATEWALKTOHIT_H_

#include "PlayerState.h"

class PlayerStateWalkToHitEnemy :	public PlayerState
{
public:
	PlayerStateWalkToHitEnemy(PlayerMovement* PM, const char* trigger);
	~PlayerStateWalkToHitEnemy();

	void Update() override;
	void CheckInput() override;
	void Enter() override;

public:
	float duration = 1.5f;
	std::vector<float3>path;
	unsigned pathIndex = 0u;
	GameObject* dustParticles = nullptr;
	GameObject* walkingEnemyTargeted = nullptr;

private:
	float moveTimer = 0.0f;
	float defaultMaxDist = 10000.f;
	float3 enemyPosition = float3(0.f, 0.f, 0.f);
	bool toAttack = false;
};

#endif // __PLAYERSTATEWALKTOHIT_H_