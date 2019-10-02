#ifndef __ENEMYSTATECHASE_H_
#define __ENEMYSTATECHASE_H_

#include "EnemyState.h"

class EnemyStateChase :
	public EnemyState
{
public:
	BasicEnemyAIScript_API EnemyStateChase(BasicEnemyAIScript* AIScript);
	BasicEnemyAIScript_API ~EnemyStateChase();

	void HandleIA() override;
	void Update() override;
private:
	math::float3 positionGoingTowards = math::float3(0.f,0.f,0.f);
};

#endif __ENEMYSTATECHASE_H_