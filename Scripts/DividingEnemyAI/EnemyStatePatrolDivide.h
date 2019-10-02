#ifndef __ENEMYSTATEPATROLDIVIDE_H_
#define __ENEMYSTATEPATROLDIVIDE_H_

#include "BasicEnemyAIScript\EnemyStatePatrol.h"
class DividingEnemyAI;

class EnemyStatePatrolDivide :
	public EnemyStatePatrol
{
public:
	EnemyStatePatrolDivide(DividingEnemyAI* AIScript);
	~EnemyStatePatrolDivide();

	void HandleIA() override;
};

#endif __ENEMYSTATEPATROLDIVIDE_H_