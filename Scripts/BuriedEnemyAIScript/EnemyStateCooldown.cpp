#include "EnemyStateCooldown.h"

#include "EnemyControllerScript.h"
#include "BuriedEnemyAIScript.h"

EnemyStateCooldown::EnemyStateCooldown(BuriedEnemyAIScript* AIScript)
{
	enemy = AIScript;
	trigger = "Cooldown";
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
			if (enemy->teleportAvailable)
			{
				enemy->currentState = (EnemyState*)enemy->relocate;
			}
			else
			{
				enemy->currentState = (EnemyState*)enemy->attack;
			}
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
			if (enemy->teleportAvailable)
			{
				enemy->currentState = (EnemyState*)enemy->relocate;
			}
		}
	}
}

void EnemyStateCooldown::Update()
{

}