#ifndef __ENEMYSTATEPATROL_H_
#define __ENEMYSTATEPATROL_H_

#include "EnemyState.h"

class EnemyStatePatrol :
	public EnemyState
{
public:
	EnemyStatePatrol(RangeEnemyAIScript* AIScript);
	~EnemyStatePatrol();

	void HandleIA() override;
	void Update() override;
};

#endif __ENEMYSTATEPATROL_H_