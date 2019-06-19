#include "EnemyStateReturnToStart.h"

#include "RangeEnemyAIScript.h"
#include "EnemyControllerScript.h"

#define START_POS_OFFSET  1.5f

EnemyStateReturnToStart::EnemyStateReturnToStart(RangeEnemyAIScript* AIScript)
{
	enemy = AIScript;
	trigger = "ReturnToStart";
}

EnemyStateReturnToStart::~EnemyStateReturnToStart()
{
}

void EnemyStateReturnToStart::Update()
{
	// Go back to start position
	enemy->enemyController->Move(enemy->returnSpeed, refreshTime, enemy->startPosition, enemyPath);

	// Check distance to player
	float distanceToPlayer = enemy->enemyController->GetDistanceToPlayer2D();
	if (distanceToPlayer < enemy->activationDistance)
	{
		enemy->currentState = (EnemyState*)enemy->getInPosition;
	}
	else 
	{
		//Check distance to start position
		// TODO: Change this with a function on nav mesh that tells you the nearest point
		float distanceToStartPosition = enemy->enemyController->GetDistanceTo2D(enemy->startPosition);
		if (distanceToStartPosition < START_POS_OFFSET)
			enemy->currentState = (EnemyState*)enemy->patrol;
	}
}
