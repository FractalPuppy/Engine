#include "EnemyStateAttack.h"

#include "EnemyControllerScript.h"
#include "FlyingSkullEnemyAIScript.h"
#include "ProjectileScript.h"

#include "GameObject.h"
#include "ComponentTransform.h"

EnemyStateAttack::EnemyStateAttack(FlyingSkullEnemyAIScript* AIScript)
{
	enemy = AIScript;
}



EnemyStateAttack::~EnemyStateAttack()
{
}

void EnemyStateAttack::HandleIA()
{
	if (timer > duration)
	{
		// End attack: Enter cooldown state
		timer = 0.f;
		enemy->currentState = (EnemyState*)enemy->cooldown;
	}
}

void EnemyStateAttack::Update()
{
	if (timer > enemy->projectileDelay && !projShot)
	{
		math::float3 playerPosition = enemy->enemyController->GetPlayerPosition();
		projShot = true;
		enemy->projectileGO->transform->SetPosition(enemy->enemyController->GetPosition() +
			math::float3(0, enemy->projectileScript->offsetHeight, 0));

		enemy->projectileGO->transform->SetRotation(enemy->enemyController->GetRotation());
		enemy->projectileGO->transform->LookAt(playerPosition + math::float3(0, enemy->projectileScript->offsetHeight, 0));
		enemy->projectileScript->shooted = true;
		projShot = true;
		enemy->projectileGO->SetActive(true);
	}
}

void EnemyStateAttack::Enter()
{
	duration = enemy->attackDuration;
	projShot = false;

	math::float3 playerPosition = enemy->enemyController->GetPlayerPosition();
	enemy->enemyController->LookAt2D(playerPosition);
}