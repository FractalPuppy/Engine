#ifndef __BOSSSTATETHIRDLEFT_H_
#define __BOSSSTATETHIRDLEFT_H_

#include "BossState.h"
#include "Math/float3.h"

class BossStateThirdLeft :
	public BossState
{
public:
	BossStateThirdLeft(BossBehaviourScript* AIBoss);
	~BossStateThirdLeft();

	void HandleIA() override;
	void Update() override;

	void Enter() override;
	void Exit() override;

	bool prefabSpawned = false;
	math::float3 GetFistPosition();
};

#endif // __BOSSSTATETHIRDLEFT_H_
