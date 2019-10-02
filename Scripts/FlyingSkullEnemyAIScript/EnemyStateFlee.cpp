#include "EnemyStateFlee.h"

#include "EnemyControllerScript.h"
#include "FlyingSkullEnemyAIScript.h"

EnemyStateFlee::EnemyStateFlee(FlyingSkullEnemyAIScript* AIScript)
{
	enemy = AIScript;
}


EnemyStateFlee::~EnemyStateFlee()
{
}

void EnemyStateFlee::HandleIA()
{
	if (timer > enemy->fleeDuration)
	{
		float distanceToPlayer = enemy->enemyController->GetDistanceToPlayer2D();

		if (distanceToPlayer > enemy->maxAttackRange)
		{
			if (distanceToPlayer > enemy->disengageDistance)
			{
				enemy->currentState = (EnemyState*)enemy->returnToStart;
			}
			else
			{
				enemy->currentState = (EnemyState*)enemy->chase;
			}
		}
		else
		{
			enemy->currentState = (EnemyState*)enemy->attack;
		}
	}
}

void EnemyStateFlee::Update()
{
	float distanceToPlayer = enemy->enemyController->GetDistanceToPlayer2D();

	math::float3 playerPos = enemy->enemyController->GetPlayerPosition();
	math::float3 enemyPos = enemy->enemyController->GetPosition();

	// Get Euclidean Vector (Vector Director) 
	float x = enemyPos.x - playerPos.x;
	float z = enemyPos.z - playerPos.z;

	// Normalize
	x /= distanceToPlayer;
	z /= distanceToPlayer;

	// Get the new point which is at maxAttackDistance from player position
	math::float3 newPosition;
	newPosition.x = ((float)playerPos.x + x * (enemy->maxAttackRange));
	newPosition.y = enemyPos.y;
	newPosition.z = ((float)playerPos.z + z * (enemy->maxAttackRange));

	// Move to that point
	enemy->enemyController->Move(enemy->runSpeed, refreshTime, newPosition, enemyPath);
}