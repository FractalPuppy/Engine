#ifndef __ENEMYSTATEATTACK_H_
#define __ENEMYSTATEATTACK_H_

#include "EnemyState.h"
class EnemyStateAttack :
	public EnemyState
{
public:
	EnemyStateAttack(FlyingSkullEnemyAIScript* AIScript);
	~EnemyStateAttack();

	void HandleIA() override;
	void Update() override;
	
	void Enter() override;
private:
	bool projShot = false;
};

#endif // __ENEMYSTATEATTACK_H_