#ifndef __ENEMYSTATEATTACK_H_
#define __ENEMYSTATEATTACK_H_

#include "EnemyState.h"

class EnemyStateAttack :
	public EnemyState
{
public:
	EnemyStateAttack(RangeEnemyAIScript* AIScript);
	~EnemyStateAttack();

	void Enter() override;
	void Update() override;
};

#endif __ENEMYSTATEATTACK_H_