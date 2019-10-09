#ifndef __ENEMYSTATECOOLDOWN_H_
#define __ENEMYSTATECOOLDOWN_H_

#include "EnemyState.h"

class EnemyStateCooldown :
	public EnemyState
{
public:
	BasicEnemyAIScript_API EnemyStateCooldown(BasicEnemyAIScript* AIScript);
	BasicEnemyAIScript_API ~EnemyStateCooldown();

	void Enter() override;

	void HandleIA() override;
	void Update() override;

private:
	float waitedTime = 1.f;
};

#endif __ENEMYSTATECOOLDOWN_H_