#ifndef __ENEMYSTATEDEATH_H_
#define __ENEMYSTATEDEATH_H_

#include "EnemyState.h"
class EnemyStateDeath :
	public EnemyState
{
public:
	EnemyStateDeath(FlyingSkullEnemyAIScript* AIScript);
	~EnemyStateDeath();

	void Enter() override;
	void HandleIA() override;
	void Update() override;

	float waitTime = 0.5f;
};

#endif // __ENEMYSTATEDEATH_H_