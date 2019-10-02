#ifndef __ENEMYSTATEHIDDEN_H_
#define __ENEMYSTATEHIDDEN_H_

#include "EnemyState.h"
class EnemyStateHidden :
	public EnemyState
{
public:
	
	void HandleIA() override;
	void Update() override;

	EnemyStateHidden(BuriedEnemyAIScript* AIScript);
	~EnemyStateHidden();
};

#endif // __ENEMYSTATEHIDDEN_H_