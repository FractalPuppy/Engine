#ifndef __ENEMYSTATECOOLDOWN_H_
#define __ENEMYSTATECOOLDOWN_H_

#include "EnemyState.h"

class EnemyStateCooldown :
	public EnemyState
{
public:
	EnemyStateCooldown(BasicEnemyAIScript* AIScript);
	~EnemyStateCooldown();

	void Update() override;
};

#endif __ENEMYSTATECOOLDOWN_H_