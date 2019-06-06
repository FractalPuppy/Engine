#pragma once
#include "EnemyState.h"
class EnemyStateDeath :
	public EnemyState
{
public:
	EnemyStateDeath(HiddenEnemyAIScript* AIScript);
	~EnemyStateDeath();

	void Enter() override;
	void Update() override;
};

