#include "EnemyStateCooldown.h"

#include "GameObject.h"
#include "Application.h"
#include "ModuleTime.h"
#include "EnemyControllerScript.h"
#include "ProjectileScript.h"
#include "RangeEnemyAIScript.h"

EnemyStateCooldown::EnemyStateCooldown(RangeEnemyAIScript* AIScript)
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
	if (timer > enemy->cooldownTime)
	{
		// Check distance to player
		if (distanceToPlayer > enemy->maxAttackDistance || distanceToPlayer < enemy->minAttackDistance)
		{
			// Get in position to attack
			enemy->currentState = (EnemyState*)enemy->chase;
		}
		else if (distanceToPlayer > enemy->disengageDistance)
		{
			// Return to start position
			enemy->currentState = (EnemyState*)enemy->returnToStart;
		}
		else if (!enemy->projectileScript1->shooted)
		{
			// Player in range and projectile is available, change to attack
			enemy->currentState = (EnemyState*)enemy->attack;
			auxTimer = 0.0f;
		}
	}
	else
	{
		if (distanceToPlayer < enemy->minAttackDistance && disengageTimer >= disengageCooldown)
		{
			enemy->currentState = (EnemyState*)enemy->flee;
			disengageTimer = 0.0f;
		}
	}
}

void EnemyStateCooldown::Update()
{
	disengageTimer += enemy->App->time->gameDeltaTime;
}
