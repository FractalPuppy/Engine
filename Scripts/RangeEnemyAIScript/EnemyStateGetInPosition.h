#ifndef __ENEMYSTATECHASE_H_
#define __ENEMYSTATECHASE_H_

#include "EnemyState.h"

class EnemyStateGetInPosition :
	public EnemyState
{
public:
	EnemyStateGetInPosition(RangeEnemyAIScript* AIScript);
	~EnemyStateGetInPosition();

	void Update() override;
};

#endif __ENEMYSTATECHASE_H_