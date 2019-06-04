#ifndef __ENEMYSTATESHOWUP_H_
#define __ENEMYSTATESHOWUP_H_

#include "EnemyState.h"

class GameObject;

class EnemyStateShowUp :
	public EnemyState
{
public:
	EnemyStateShowUp(HiddenEnemyAIScript* AIScript);
	~EnemyStateShowUp();

	void Update() override;

public:
	GameObject* unborrowParticles = nullptr;
};

#endif __ENEMYSTATESHOWUP_H_