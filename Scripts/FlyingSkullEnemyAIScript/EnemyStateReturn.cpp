#include "EnemyStateReturn.h"

#include "EnemyControllerScript.h"
#include "FlyingSkullEnemyAIScript.h"

#define START_POS_OFFSET  20.f

EnemyStateReturn::EnemyStateReturn(FlyingSkullEnemyAIScript* AIScript)
{
	enemy = AIScript;
}


EnemyStateReturn::~EnemyStateReturn()
{
}

void EnemyStateReturn::HandleIA()
{
	// Check distance to player
	float distanceToPlayer = enemy->enemyController->GetDistanceToPlayer2D();
	if (distanceToPlayer < enemy->activationDistance)
	{
		enemy->currentState = (EnemyState*)enemy->chase;
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

void EnemyStateReturn::Update()
{
	// Go back to start position
	enemy->enemyController->Move(enemy->runSpeed, refreshTime, enemy->startPosition, enemyPath);
}