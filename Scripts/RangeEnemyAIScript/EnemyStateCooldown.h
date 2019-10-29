#ifndef __ENEMYSTATECOOLDOWN_H_
#define __ENEMYSTATECOOLDOWN_H_

#include "EnemyState.h"

class EnemyStateCooldown :
	public EnemyState
{
public:
	EnemyStateCooldown(RangeEnemyAIScript* AIScript);
	~EnemyStateCooldown();

	void HandleIA() override;
	void Update() override;

	float disengageCooldown = 2.0f;
	float disengageTimer = 2.0f;
};

#endif __ENEMYSTATECOOLDOWN_H_