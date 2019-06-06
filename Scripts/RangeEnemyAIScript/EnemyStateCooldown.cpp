#include "EnemyStateCooldown.h"

#include "GameObject.h"

#include "EnemyControllerScript.h"
#include "RangeEnemyAIScript.h"

EnemyStateCooldown::EnemyStateCooldown(RangeEnemyAIScript* AIScript)
{
	enemy = AIScript;
	trigger = "Cooldown";
}


EnemyStateCooldown::~EnemyStateCooldown()
{
}

void EnemyStateCooldown::Update()
{
	if (timer > enemy->cooldownTime)
	{
		// Check distance to player
		float distanceToPlayer = enemy->enemyController->GetDistanceToPlayer2D();
		if (distanceToPlayer > enemy->maxAttackDistance || distanceToPlayer < enemy->minAttackDistance)
		{
			// Player in range, change to attack
			enemy->currentState = (EnemyState*)enemy->getInPosition;
		}
		else if (distanceToPlayer > enemy->disengageDistance)
		{
			// Return to start position
			enemy->currentState = (EnemyState*)enemy->returnToStart;
		}
		else if(!enemy->projectile->isActive())
		{
			enemy->currentState = (EnemyState*)enemy->attack;
			auxTimer = 0.0f;
		}
	}
}
