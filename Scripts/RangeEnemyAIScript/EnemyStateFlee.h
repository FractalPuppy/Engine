#ifndef __ENEMYSTATEFLEE_H_
#define __ENEMYSTATEFLEE_H_

#include "EnemyState.h"

class EnemyStateFlee :
	public EnemyState
{
public:
	EnemyStateFlee(RangeEnemyAIScript* AIScript);
	~EnemyStateFlee();

	void HandleIA() override;
	void Update() override;

	void Enter() override;
	void Exit() override;
	void FindFleeDirection();

private:
	void MoveAwayFromPlayer();
	void ChangeDirection();

private:
	math::float3 fleeDestiny;
	math::float3 enemyPos;
	math::float3 fleeDir;
	math::float3 playerPos;
	int changedTimes = 0;
	bool skipFlee = false;

};

#endif // __ENEMYSTATEFLEE_H_