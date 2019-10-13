#include "EnemyStateAttack.h"

#include "BuriedEnemyAIScript.h"
#include "EnemyControllerScript.h"
#include "ProjectileScript.h"

#include "GameObject.h"
#include "ComponentTransform.h"
#include "Math/float3.h"

#include "assert.h"

EnemyStateAttack::EnemyStateAttack(BuriedEnemyAIScript* AIScript)
{
	enemy = AIScript;
	trigger = "Attack";
}


EnemyStateAttack::~EnemyStateAttack()
{
}

void EnemyStateAttack::HandleIA()
{
	if (timer > duration)
	{
		// End attack: Enter cooldown state
		//but first we make sure it does not move while on cooldown
		enemy->enemyController->Move(enemy->runSpeed, refreshTime, enemy->enemyController->GetPosition(), enemyPath);
		//also that it keeps looking at the player
		enemy->enemyController->gameobject->transform->LookAt(enemy->enemyController->GetPlayerPosition());
		//now switches state
		timer = 0.f;
		enemy->currentState = (EnemyState*)enemy->cooldown;
	}
}

void EnemyStateAttack::Update()
{
	//Make sure the delay makes sense
	assert(enemy->projectileDelay < duration);

	math::float3 playerPosition = enemy->enemyController->GetPlayerPosition();
	enemy->enemyController->LookAt2D(playerPosition + math::float3(0, enemy->projectileScript->offsetHeight, 0));

	if (timer > enemy->projectileDelay && !projShot)
	{
		math::float3 playerPosition = enemy->enemyController->GetPlayerPosition();

		enemy->projectileScript->CleanTrailFX();
		math::float3 dir = (enemy->enemyController->GetPosition() - playerPosition).Normalized();
		enemy->projectileScript->direction = math::float3(dir.x, 0, dir.z);

		enemy->projectileGO->transform->SetGlobalPosition(enemy->enemyController->GetPosition() + 
			math::float3(0, enemy->projectileScript->offsetHeight, 0));
		enemy->projectileGO->transform->SetRotation(enemy->enemyController->GetRotation());

		enemy->projectileScript->shooted = true;
		projShot = true;
		enemy->projectileGO->SetActive(true);
	}
}

void EnemyStateAttack::Enter()
{
	projShot = false;
}

void EnemyStateAttack::Exit()
{
}