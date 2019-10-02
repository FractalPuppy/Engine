#ifndef __ENEMYSTATECOOLDOWN_H_
#define __ENEMYSTATECOOLDOWN_H_

#include "EnemyState.h"
class EnemyStateCooldown :
	public EnemyState
{
public:
	EnemyStateCooldown(BuriedEnemyAIScript* AIScript);
	~EnemyStateCooldown();

	void HandleIA() override;
	void Update() override;
};

#endif // __ENEMYSTATECOOLDOWN_H_