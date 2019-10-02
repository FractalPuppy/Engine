#ifndef __ENEMYSTATERELOCATE_H_
#define __ENEMYSTATERELOCATE_H_

#include "EnemyState.h"
class EnemyStateRelocate :
	public EnemyState
{
public:
	EnemyStateRelocate(BuriedEnemyAIScript* AIScript);
	~EnemyStateRelocate();

	void HandleIA() override;
	void Update() override;

	void Exit() override;
	void Enter() override;

	void ChangePosition();

	bool finished = false;
};

#endif // __ENEMYSTATERELOCATE_H_