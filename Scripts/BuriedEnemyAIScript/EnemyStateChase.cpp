#include "EnemyStateChase.h"

#include "BuriedEnemyAIScript.h"
#include "EnemyControllerScript.h"

EnemyStateChase::EnemyStateChase(BuriedEnemyAIScript* AIScript)
{
	enemy = AIScript;
	trigger = "Chase";
}


EnemyStateChase::~EnemyStateChase()
{
}

void EnemyStateChase::HandleIA()
{
	float distanceToPlayer = enemy->enemyController->GetDistanceToPlayer2D();

	if (distanceToPlayer > enemy->disengageDistance)
	{
		enemy->currentState = (EnemyState*)enemy->returnToStart;
	}
	else if (distanceToPlayer < enemy->maxAttackRange && distanceToPlayer > enemy->minAttackRange)
	{
		enemy->currentState = (EnemyState*)enemy->attack;
		enemy->enemyController->Stop();
	}
	else if (distanceToPlayer < enemy->minAttackRange && enemy->teleportAvailable)
	{
		enemy->currentState = (EnemyState*)enemy->relocate;
	}
	else if (distanceToPlayer < enemy->minAttackRange && !enemy->teleportAvailable)
	{
		enemy->currentState = (EnemyState*)enemy->attack;
	}

}

void EnemyStateChase::Update()
{
	// Move towards the player
	enemy->enemyController->Move(enemy->runSpeed, refreshTime, enemy->enemyController->GetPlayerPosition(), enemyPath);
}