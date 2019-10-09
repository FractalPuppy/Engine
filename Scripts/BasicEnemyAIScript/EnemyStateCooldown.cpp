#include "EnemyStateCooldown.h"

#include "BasicEnemyAIScript.h"
#include "EnemyControllerScript.h"

EnemyStateCooldown::EnemyStateCooldown(BasicEnemyAIScript* AIScript)
{
	enemy = AIScript;
	trigger = "Cooldown";
}


EnemyStateCooldown::~EnemyStateCooldown()
{
}

void EnemyStateCooldown::Enter()
{
	enemy->enemyController->LookAt2D(enemy->enemyController->GetPlayerPosition());
}

void EnemyStateCooldown::HandleIA()
{
	float distance = enemy->enemyController->GetDistanceToPlayer2D();

	if (enemy->scared)
	{
		enemy->currentState = (EnemyState*)enemy->flee;
	}
	else if (distance > enemy->attackRange)
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
	waitedTime = (timer - auxTimer);

}
