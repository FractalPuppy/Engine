#ifndef __BOSSSTATETHIRDEATH_H_
#define __BOSSSTATETHIRDEATH_H_

#include "BossState.h"
class BossStateThirdDeath :
	public BossState
{
public:
	BossStateThirdDeath(BossBehaviourScript* AIBoss);
	~BossStateThirdDeath();

	void HandleIA() override;
	void Update() override;

	void Enter() override;
	void Exit() override;

	float deathTimeout = 5.0f;
};

#endif