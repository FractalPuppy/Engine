#ifndef __ENEMYSTATERETURNTOSTART_H_
#define __ENEMYSTATERETURNTOSTART_H_

#include "EnemyState.h"
class EnemyStateReturnToStart :
	public EnemyState
{
public:
	EnemyStateReturnToStart(HiddenEnemyAIScript* AIScript);
	~EnemyStateReturnToStart();

	void Update() override;
};

#endif __ENEMYSTATERETURNTOSTART_H_