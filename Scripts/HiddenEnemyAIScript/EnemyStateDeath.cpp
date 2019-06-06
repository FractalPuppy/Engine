#include "EnemyStateDeath.h"

#include "GameObject.h"
#include "HiddenEnemyAIScript.h"

EnemyStateDeath::EnemyStateDeath(HiddenEnemyAIScript* AIScript)
{
	enemy = AIScript;
	trigger = "Death";
}


EnemyStateDeath::~EnemyStateDeath()
{
}

void EnemyStateDeath::Enter()
{
	auxTimer = 0.0f;
}

void EnemyStateDeath::Update()
{
	float waitedTime = (timer - auxTimer);

	if (waitedTime > 10.0f)
	{
		enemy->gameobject->SetActive(false);
	}
}
