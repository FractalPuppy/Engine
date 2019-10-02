#include "EnemyStateCooldown.h"

#include "EnemyControllerScript.h"
#include "FlyingSkullEnemyAIScript.h"

EnemyStateCooldown::EnemyStateCooldown(FlyingSkullEnemyAIScript* AIScript)
{
	enemy = AIScript;
}


EnemyStateCooldown::~EnemyStateCooldown()
{
}

void EnemyStateCooldown::HandleIA()
{
	float distanceToPlayer = enemy->enemyController->GetDistanceToPlayer2D();

	if (timer > enemy->cooldownDuration)
	{
		if (distanceToPlayer < enemy->maxAttackRange && distanceToPlayer > enemy->minAttackRange)
		{
			enemy->currentState = (EnemyState*)enemy->attack;
		}
		else if (distanceToPlayer < enemy->disengageDistance && distanceToPlayer > enemy->maxAttackRange)
		{
			enemy->currentState = (EnemyState*)enemy->chase;
		}
		else if (distanceToPlayer < enemy->minAttackRange)
		{
			enemy->currentState = (EnemyState*)enemy->flee;
		}
	}
	else
	{
		if (distanceToPlayer > enemy->disengageDistance)
		{
			enemy->currentState = (EnemyState*)enemy->returnToStart;
		}
		else if (distanceToPlayer < enemy->disengageDistance && distanceToPlayer > enemy->maxAttackRange)
		{
			enemy->currentState = (EnemyState*)enemy->chase;
		}
		else if (distanceToPlayer < enemy->minAttackRange)
		{
			enemy->currentState = (EnemyState*)enemy->flee;
		}
	}


}

void EnemyStateCooldown::Update()
{
	math::float3 playerPosition = enemy->enemyController->GetPlayerPosition();
	enemy->enemyController->LookAt2D(playerPosition);
}
