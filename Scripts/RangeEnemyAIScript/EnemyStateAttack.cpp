#include "EnemyStateAttack.h"

#include "GameObject.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleTime.h"

#include "ComponentTransform.h"
#include "ComponentBoxTrigger.h"

#include "EnemyControllerScript.h"
#include "ProjectileScript.h"
#include "RangeEnemyAIScript.h"

EnemyStateAttack::EnemyStateAttack(RangeEnemyAIScript* AIScript)
{
	enemy = AIScript;
	trigger = "Attack";

	gunFX = enemy->App->scene->FindGameObjectByName("GunFX", enemy->gameobject);
}

EnemyStateAttack::~EnemyStateAttack()
{
}

void EnemyStateAttack::Enter()
{
	ShowGunFX();
	projShot1 = false;
	projShot2 = false;
	projShot3 = false;

	//make the enemy stop moving
	enemy->enemyController->Stop();

	duration = duration * enemy->numberOfProjectiles;
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
	ProcessGunFx();

	// Delay attack
	if (timer > enemy->projectileDelay1 && !projShot1)
	{
		// Reset projectile position
		math::float3 dir = (enemy->enemyController->GetPosition() - enemy->enemyController->
		GetPlayerPosition()).Normalized();
		enemy->projectileScript1->direction = math::float3(dir.x, 0, dir.z);
		enemy->projectileScript1->CleanTrailFX();

		enemy->projectile1->transform->SetGlobalPosition(enemy->enemyController->GetPosition() + math::float3(0.f,enemy->projectileScript1->offsetHeight,0.f));
		enemy->projectile1->transform->SetGlobalRotation(enemy->enemyController->GetRotation());
		enemy->projectileScript1->shooted = true;
		projShot1 = true;
		enemy->projectile1->SetActive(true);
	}
	if (enemy->numberOfProjectiles > 1 && timer > enemy->projectileDelay2 && !projShot2)
	{
		ShowGunFX();
		// Reset projectile position
		math::float3 dir = (enemy->enemyController->GetPosition() - enemy->enemyController->
			GetPlayerPosition()).Normalized();
		enemy->projectileScript2->direction = math::float3(dir.x, 0, dir.z);
		enemy->projectileScript2->CleanTrailFX();

		enemy->projectile2->transform->SetGlobalPosition(enemy->enemyController->GetPosition() + math::float3(0.f, enemy->projectileScript2->offsetHeight, 0.f));
		enemy->projectile2->transform->SetGlobalRotation(enemy->enemyController->GetRotation());
		enemy->projectileScript2->shooted = true;
		projShot2 = true;
		enemy->projectile2->SetActive(true);
	}
	if (enemy->numberOfProjectiles > 2 && timer > enemy->projectileDelay3 && !projShot3)
	{
		ShowGunFX();
		// Reset projectile position
		math::float3 dir = (enemy->enemyController->GetPosition() - enemy->enemyController->
			GetPlayerPosition()).Normalized();
		enemy->projectileScript3->direction = math::float3(dir.x, 0, dir.z);
		enemy->projectileScript3->CleanTrailFX();

		enemy->projectile3->transform->SetGlobalPosition(enemy->enemyController->GetPosition() + math::float3(0.f, enemy->projectileScript3->offsetHeight, 0.f));
		enemy->projectile3->transform->SetGlobalRotation(enemy->enemyController->GetRotation());
		enemy->projectileScript3->shooted = true;
		projShot3 = true;
		enemy->projectile3->SetActive(true);
	}
	
	// Keep looking at player
	math::float3 playerPosition = enemy->enemyController->GetPlayerPosition();
	enemy->enemyController->LookAt2D(playerPosition);
}

void EnemyStateAttack::ShowGunFX()
{
	gunFX->SetActive(true);
	gunTimer = 0.0f;
}

void EnemyStateAttack::ProcessGunFx()
{
	gunTimer += enemy->App->time->gameDeltaTime;
	while (gunTimer >= gunTotalTimer)
	{
		gunFX->SetActive(false);
		gunTimer = 0.0f;
	}
}