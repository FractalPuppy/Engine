#ifndef __ENEMYSTATEPATROL_H_
#define __ENEMYSTATEPATROL_H_

#include "EnemyState.h"

class EnemyStatePatrol :
	public EnemyState
{
public:
	BasicEnemyAIScript_API EnemyStatePatrol(BasicEnemyAIScript* AIScript);
	BasicEnemyAIScript_API ~EnemyStatePatrol();

	BasicEnemyAIScript_API void HandleIA() override;
	BasicEnemyAIScript_API void Update() override;
};

#endif __ENEMYSTATEPATROL_H_