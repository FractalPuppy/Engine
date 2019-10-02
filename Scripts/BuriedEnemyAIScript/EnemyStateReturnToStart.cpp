#include "EnemyStateReturnToStart.h"

#include "BuriedEnemyAIScript.h"
#include "EnemyControllerScript.h"

#define START_POS_OFFSET  25.0f

EnemyStateReturnToStart::EnemyStateReturnToStart(BuriedEnemyAIScript* AIScript)
{
	enemy = AIScript;
	trigger = "Return";
}


EnemyStateReturnToStart::~EnemyStateReturnToStart()
{
}

void EnemyStateReturnToStart::HandleIA()
{
	float distanceToPlayer = enemy->enemyController->GetDistanceToPlayer2D();
	if (distanceToPlayer < enemy->activationDistance)
	{
		enemy->currentState = (EnemyState*)enemy->chase;
	}
	else
	{
		float distanceToStartPosition = enemy->enemyController->GetDistanceTo2D(enemy->startPosition);
		if (distanceToStartPosition < START_POS_OFFSET)
			enemy->currentState = (EnemyState*)enemy->hide;
	}
}

void EnemyStateReturnToStart::Update()
{
	// Move back to start position
	enemy->enemyController->Move(enemy->runSpeed, refreshTime, enemy->startPosition, enemyPath);
}