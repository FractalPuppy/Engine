#include "EnemyStatePatrolDivide.h"
#include "BasicEnemyAIScript/BasicEnemyAIScript.h"
#include "EnemyControllerScript/EnemyControllerScript.h"
#include "DividingEnemyAI.h"

EnemyStatePatrolDivide::EnemyStatePatrolDivide(DividingEnemyAI* AIScript) : EnemyStatePatrol((BasicEnemyAIScript*) AIScript)
{
}

EnemyStatePatrolDivide::~EnemyStatePatrolDivide()
{
}

void EnemyStatePatrolDivide::HandleIA()
{
	float distanceToPlayer = enemy->enemyController->GetDistanceToPlayer2D();

	if (distanceToPlayer < enemy->activationDistance)
	{
		enemy->currentState = (EnemyState*)((DividingEnemyAI*)enemy)->multiply;
	}
}
