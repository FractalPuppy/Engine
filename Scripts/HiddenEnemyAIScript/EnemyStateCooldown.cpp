#include "EnemyStateCooldown.h"

#include "HiddenEnemyAIScript.h"
#include "EnemyControllerScript.h"

EnemyStateCooldown::EnemyStateCooldown(HiddenEnemyAIScript* AIScript)
{
	enemy = AIScript;
	trigger = "Cooldown";
}


EnemyStateCooldown::~EnemyStateCooldown()
{
}

void EnemyStateCooldown::HandleIA()
{
	float distance = enemy->enemyController->GetDistanceToPlayer2D();

	if (distance > enemy->attackRange)
	{
		enemy->currentState = (EnemyState*)enemy->chase;
	}
	else if (timer > enemy->cooldownTime)
	{
		enemy->currentState = (EnemyState*)enemy->attack;
		auxTimer = 0.0f;
	}
}

void EnemyStateCooldown::Update()
{
	float waitedTime = (timer - auxTimer);
}
