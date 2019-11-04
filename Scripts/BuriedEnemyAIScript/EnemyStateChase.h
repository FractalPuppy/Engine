#ifndef __ENEMYSTATECHASE_H_
#define __ENEMYSTATECHASE_H_

#include "EnemyState.h"
class EnemyStateChase :
	public EnemyState
{
public:
	EnemyStateChase(BuriedEnemyAIScript* AIScript);
	~EnemyStateChase();

	void HandleIA() override;
	void Update() override;
	void Exit() override;

	float timer = 1.0f;
	float walkTimer = 0.50f;
};

#endif // __ENEMYSTATECHASE_H_

