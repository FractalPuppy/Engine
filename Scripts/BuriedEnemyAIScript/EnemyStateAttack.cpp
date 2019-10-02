#include "EnemyStateAttack.h"

#include "BuriedEnemyAIScript.h"
#include "EnemyControllerScript.h"
#include "ProjectileScript.h"

#include "GameObject.h"
#include "ComponentTransform.h"

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

	//enemy->enemyController->LookAt(enemy->enemyController->GetPlayerPosition());

}

void EnemyStateAttack::Enter()
{
	projShot = false;

	math::float3 playerPosition = enemy->enemyController->GetPlayerPosition();
	enemy->enemyController->LookAt2D(playerPosition);
}

void EnemyStateAttack::Exit()
{

}