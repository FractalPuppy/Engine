#include "EnemyStateAttack.h"

#include "GameObject.h"

#include "ComponentTransform.h"
#include "ComponentBoxTrigger.h"

#include "EnemyControllerScript.h"
#include "RangeEnemyAIScript.h"

EnemyStateAttack::EnemyStateAttack(RangeEnemyAIScript* AIScript)
{
	enemy = AIScript;
	trigger = "Attack";
}

EnemyStateAttack::~EnemyStateAttack()
{
}

void EnemyStateAttack::Enter()
{
	projectileShooted = false;
}

void EnemyStateAttack::Update()
{
	if (!projectileShooted && !enemy->projectile->isActive())
	{
		// Delay attack
		if (timer > enemy->projectileDelay)
		{
			// Reset projectile position
			enemy->projectile->transform->SetPosition(enemy->enemyController->GetPosition());
			enemy->projectile->transform->SetRotation(enemy->enemyController->GetRotation());

			enemy->projectile->SetActive(true);
			projectileShooted = true;
		}
	}

	// Keep looking at player
	math::float3 playerPosition = enemy->enemyController->GetPlayerPosition();
	enemy->enemyController->LookAt2D(playerPosition);

	if (timer > enemy->attackDuration && projectileShooted)
	{
		// End attack: Enter cooldown state
		auxTimer = timer;
		enemy->currentState = (EnemyState*)enemy->cooldown;
	}
}