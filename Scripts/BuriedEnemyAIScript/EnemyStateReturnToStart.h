#ifndef __ENEMYSTATERETURN_H_
#define __ENEMYSTATERETURN_H_

#include "EnemyState.h"
class EnemyStateReturnToStart :
	public EnemyState
{
public:
	EnemyStateReturnToStart(BuriedEnemyAIScript* AIScript);
	~EnemyStateReturnToStart();

	void HandleIA() override;
	void Update() override;

};

#endif // __ENEMYSTATERETURN_H_