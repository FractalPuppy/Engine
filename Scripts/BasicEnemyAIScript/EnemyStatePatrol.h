#ifndef __ENEMYSTATEPATROL_H_
#define __ENEMYSTATEPATROL_H_

#include "EnemyState.h"

class EnemyStatePatrol :
	public EnemyState
{
public:
	EnemyStatePatrol(BasicEnemyAIScript* AIScript);
	~EnemyStatePatrol();

	void Update() override;
};

#endif __ENEMYSTATEPATROL_H_