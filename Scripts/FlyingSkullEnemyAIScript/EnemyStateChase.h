#ifndef __ENEMYSTATECHASE_H_
#define __ENEMYSTATECHASE_H_


#include "EnemyState.h"
class EnemyStateChase :
	public EnemyState
{
public:
	EnemyStateChase(FlyingSkullEnemyAIScript* AIScript);
	~EnemyStateChase();

	void HandleIA() override;
	void Update() override;

	void ApproachPlayer();
};

#endif // __ENEMYSTATECHASE_H_