#pragma once
#include "EnemyState.h"
class EnemyStateDeath :
	public EnemyState
{
public:
	EnemyStateDeath(BasicEnemyAIScript* AIScript);
	~EnemyStateDeath();

	void Enter() override;
	void Update() override;
};

