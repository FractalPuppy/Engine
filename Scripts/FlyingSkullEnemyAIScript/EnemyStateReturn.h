#ifndef __ENEMYSTATERETURN_H_
#define __ENEMYSTATERETURN_H_

#include "EnemyState.h"

class EnemyStateReturn : public EnemyState
{
public:
	EnemyStateReturn(FlyingSkullEnemyAIScript* AIScript);
	~EnemyStateReturn();

	void HandleIA() override;
	void Update() override;
};

#endif // __ENEMYSTATERETURN_H_