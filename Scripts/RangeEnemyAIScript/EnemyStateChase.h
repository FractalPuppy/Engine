#ifndef __ENEMYSTATECHASE_H_
#define __ENEMYSTATECHASE_H_

#include "EnemyState.h"

class EnemyStateChase :
	public EnemyState
{
public:
	EnemyStateChase(RangeEnemyAIScript* AIScript);
	~EnemyStateChase();

	void HandleIA() override;
	void Update() override;

private:
	void AproachPlayer();
	float timer = 1.0f;
	float walkTimer = 0.4f;
};

#endif __ENEMYSTATECHASE_H_