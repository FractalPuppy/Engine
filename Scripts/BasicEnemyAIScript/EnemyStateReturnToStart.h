#ifndef __ENEMYSTATERETURNTOSTART_H_
#define __ENEMYSTATERETURNTOSTART_H_

#include "EnemyState.h"
class EnemyStateReturnToStart :
	public EnemyState
{
public:
	BasicEnemyAIScript_API EnemyStateReturnToStart(BasicEnemyAIScript* AIScript);
	BasicEnemyAIScript_API ~EnemyStateReturnToStart();

	void HandleIA() override;
	void Update() override;
	void Enter() override;
};

#endif __ENEMYSTATERETURNTOSTART_H_