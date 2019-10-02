#ifndef __ENEMYSTATEHIDE_H_
#define __ENEMYSTATEHIDE_H_

#include "EnemyState.h"
class EnemyStateHide :
	public EnemyState
{
public:
	EnemyStateHide(BuriedEnemyAIScript* AIScript);
	~EnemyStateHide();

	void HandleIA() override;
	void Update() override;

	void Enter() override;
	void Exit() override;
};

#endif // __ENEMYSTATEHIDE_H_