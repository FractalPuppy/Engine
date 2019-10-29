#ifndef __ENEMYSTATEPATROL_H_
#define __ENEMYSTATEPATROL_H_

#include "EnemyState.h"

class EnemyStatePatrol :
	public EnemyState
{
public:
	EnemyStatePatrol(RangeEnemyAIScript* AIScript);
	~EnemyStatePatrol();

	void HandleIA() override;
	void Enter() override;
	void Update() override;

private:
	float patrolDistance = 300.0f;
	math::float3 startPos, endPos, destiny;
};

#endif __ENEMYSTATEPATROL_H_