#ifndef __ENEMYSTATEFLEE_H_
#define __ENEMYSTATEFLEE_H_

#include "EnemyState.h"
class EnemyStateFlee :
	public EnemyState
{
public:
	EnemyStateFlee(FlyingSkullEnemyAIScript* AIScript);
	~EnemyStateFlee();

	void HandleIA() override;
	void Update() override;
};

#endif // __ENEMYSTATEFLEE_H_