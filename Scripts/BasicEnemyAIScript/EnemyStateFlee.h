#ifndef __ENEMYSTATEFLEE_H_
#define __ENEMYSTATEFLEE_H_

#include "EnemyState.h"
class EnemyStateFlee :
	public EnemyState
{
public:
	BasicEnemyAIScript_API EnemyStateFlee(BasicEnemyAIScript* AIScript);
	BasicEnemyAIScript_API ~EnemyStateFlee();

	void Enter() override;
	void HandleIA() override;
	void Update() override;

private:
	void MoveAwayFromPlayer();
};

#endif // __ENEMYSTATEFLEE_H_