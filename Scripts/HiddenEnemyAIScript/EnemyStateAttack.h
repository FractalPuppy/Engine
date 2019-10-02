#ifndef __ENEMYSTATEATTACK_H_
#define __ENEMYSTATEATTACK_H_

#include "EnemyState.h"

class EnemyStateAttack :
	public EnemyState
{
public:
	EnemyStateAttack(HiddenEnemyAIScript* AIScript);
	~EnemyStateAttack();

	void HandleIA() override;
	void Update() override;

private:
	void Attack();
	bool attacked = false;
};

#endif __ENEMYSTATEATTACK_H_