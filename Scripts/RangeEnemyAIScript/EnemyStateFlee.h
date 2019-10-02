#ifndef __ENEMYSTATEFLEE_H_
#define __ENEMYSTATEFLEE_H_

#include "EnemyState.h"

class EnemyStateFlee :
	public EnemyState
{
public:
	EnemyStateFlee(RangeEnemyAIScript* AIScript);
	~EnemyStateFlee();

	void HandleIA() override;
	void Update() override;

	void Enter() override;

private:
	void MoveAwayFromPlayer();
};

#endif // __ENEMYSTATEFLEE_H_