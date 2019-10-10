#ifndef __BOSSSTATESUMMONARMY_H_
#define __BOSSSTATESUMMONARMY_H_

#include "BossState.h"
#include "Math/float3.h"
#include <vector>

class EnemyControllerScript;

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
	float timerSkeletons = 0.0f;
	bool AllEnemiesAppeared();
	
};

#endif // __BOSSSTATESUMMONARMY_H_
