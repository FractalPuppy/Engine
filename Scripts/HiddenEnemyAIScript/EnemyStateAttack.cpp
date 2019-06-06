#include "EnemyStateAttack.h"

#include "GameObject.h"

#include "ComponentTransform.h"
#include "ComponentBoxTrigger.h"

#include "HiddenEnemyAIScript.h"
#include "EnemyControllerScript.h"

EnemyStateAttack::EnemyStateAttack(HiddenEnemyAIScript* AIScript)
{
	enemy = AIScript;
	trigger = "Attack";
	boxSize = math::float3(100.f, 50.f, 50.f);
	minTime = 0.7f;
	maxTime = 0.9f;
}

EnemyStateAttack::~EnemyStateAttack()
{
}

void EnemyStateAttack::Update()
{
	// Keep looking at player
	math::float3 playerPosition = enemy->enemyController->GetPlayerPosition();
	enemy->enemyController->LookAt2D(playerPosition);

	if (!enemy->enemyController->IsCollidingWithPlayer())
	{
		if (hitboxCreated)
		{
			// Disable hitbox
			enemy->enemyController->attackBoxTrigger->Enable(false);
			hitboxCreated = false;
		}
		enemy->currentState = (EnemyState*)enemy->chase;
	}
	else
	{
		assert(enemy->enemyController->attackBoxTrigger != nullptr);
		if (!hitboxCreated)
		{
			// Attack
			//Create the hitbox
			enemy->enemyController->attackBoxTrigger->Enable(true);
			enemy->enemyController->attackBoxTrigger->SetBoxSize(boxSize);
			boxPosition = enemy->gameobject->transform->up * 100.f;
			enemy->enemyController->attackBoxTrigger->SetBoxPosition(boxPosition.x, boxPosition.y, boxPosition.z + 100.f);
			hitboxCreated = true;

			//playerScript->Damage(attackDamage);
			auxTimer = timer;
		}
		else if (timer > auxTimer + enemy->attackDuration)
		{
			// End attack
			// Disable hitbox
			enemy->enemyController->attackBoxTrigger->Enable(false);
			hitboxCreated = false;

			// Enter cooldown state
			auxTimer = timer;
			enemy->currentState = (EnemyState*)enemy->cooldown;
		}
	}
}