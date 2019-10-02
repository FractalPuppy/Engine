#ifndef __ENEMYSTATESHOWUP_H_
#define __ENEMYSTATESHOWUP_H_

#include "EnemyState.h"
class EnemyStateShowUp :
	public EnemyState
{
public:
	EnemyStateShowUp(BuriedEnemyAIScript* AIScript);
	~EnemyStateShowUp();

	void HandleIA() override;
	void Update() override;
	void Enter() override;
	void Exit() override;
};

#endif // __ENEMYSTATESHOWUP_H_