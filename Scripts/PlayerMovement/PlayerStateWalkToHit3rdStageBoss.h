#ifndef __PLAYERSTATEWALKTOHIT3BOSS_H_
#define __PLAYERSTATEWALKTOHIT3BOSS_H_

#include "PlayerState.h"

class PlayerStateWalkToHit3rdStageBoss :	public PlayerState
{
public:
	PlayerStateWalkToHit3rdStageBoss(PlayerMovement* PM, const char* trigger);
	~PlayerStateWalkToHit3rdStageBoss();

	void Update() override;
	void CheckInput() override;
	void Enter() override;

public:
	float duration = 1.5f;
	std::vector<float3>path;
	unsigned pathIndex = 0u;
	GameObject* dustParticles = nullptr;
	GameObject* walkingEnemyTargeted = nullptr;
	float targetBoxWidth = 0.0f;

private:
	float timeBetweenMoveCalls = 0.5f;
	float currentTime = 0.0f;
	float defaultMaxDist = 10000.f;
	float3 enemyPosition = float3(0.f, 0.f, 0.f);
	bool toAttack = false;
};

#endif // __PLAYERSTATEWALKTOHIT_H_