#ifndef __ENEMYSTATERETURNTOSTART_H_
#define __ENEMYSTATERETURNTOSTART_H_

#include "EnemyState.h"
class EnemyStateReturnToStart :
	public EnemyState
{
public:
	EnemyStateReturnToStart(RangeEnemyAIScript* AIScript);
	~EnemyStateReturnToStart();

	void HandleIA() override;
	void Update() override;
};

#endif __ENEMYSTATERETURNTOSTART_H_