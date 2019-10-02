#ifndef __ENEMYSTATECOOLDOWN_H_
#define __ENEMYSTATECOOLDOWN_H_

#include "EnemyState.h"

class EnemyStateCooldown :
	public EnemyState
{
public:
	EnemyStateCooldown(HiddenEnemyAIScript* AIScript);
	~EnemyStateCooldown();

	void HandleIA() override;
	void Update() override;

private:
	float waitedTime = 0.f;
};

#endif __ENEMYSTATECOOLDOWN_H_