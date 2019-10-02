#ifndef __ENEMYSTATEWAIT_H_
#define __ENEMYSTATEWAIT_H_

#include "EnemyState.h"

class EnemyStateWait :
	public EnemyState
{
public:
	EnemyStateWait(HiddenEnemyAIScript* AIScript);
	~EnemyStateWait();

	void HandleIA() override;
	void Update() override;
};

#endif __ENEMYSTATEWAIT_H_