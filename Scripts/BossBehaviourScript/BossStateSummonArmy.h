#ifndef __BOSSSTATESUMMONARMY_H_
#define __BOSSSTATESUMMONARMY_H_

#include "BossState.h"
#include "Math/float3.h"
#include <vector>

class EnemyControllerScript;

enum class animationState
{
	None,
	Precast,
	Cast,
	Finished
};

class BossStateSummonArmy :
	public BossState
{
public:
	BossStateSummonArmy(BossBehaviourScript* AIBoss);
	~BossStateSummonArmy();

	void HandleIA() override;
	void Update() override;

	void Enter() override;
	void Exit() override;

private:
	int enemiesSpawned = 0;

	float downTime = 8.0f; //time until she starts summoning
	bool AllEnemiesAppeared();
	bool firstSummon = false;

	bool casted = false;

	animationState animState = animationState::None;

	float animTimer = 0.0f;
	float animDuration = 0.0f;
	float colorTimer = 0.0f;
	float timerSkeletons = 0.0f;

	void LerpFogColor();

	math::float3 initialColor = math::float3::zero;
};

#endif // __BOSSSTATESUMMONARMY_H_
