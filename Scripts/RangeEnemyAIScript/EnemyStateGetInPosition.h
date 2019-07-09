#ifndef __ENEMYSTATECHASE_H_
#define __ENEMYSTATECHASE_H_

#include "EnemyState.h"

class EnemyStateGetInPosition :
	public EnemyState
{
public:
	EnemyStateGetInPosition(RangeEnemyAIScript* AIScript);
	~EnemyStateGetInPosition();

	void HandleIA() override;
	void Update() override;

private:
	void AproachPlayer();
	void MoveAwayFromPlayer();
};

#endif __ENEMYSTATECHASE_H_