#include "EnemyStateMultiply.h"
#include "BasicEnemyAIScript/BasicEnemyAIScript.h"
#include "EnemyControllerScript/EnemyControllerScript.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleScript.h"
#include "ModuleTime.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentRenderer.h"

#define ENEMY_PREFAB "BasicEnemy"

EnemyStateMultiply::EnemyStateMultiply(BasicEnemyAIScript* AIScript)
{
	enemy = AIScript;
	trigger = "Chase"; //No need to change State Machine ATM
	spawnParticleSystems = enemy->App->scene->FindGameObjectsByTag("SpawnParticles", enemy->gameobject);
}

EnemyStateMultiply::~EnemyStateMultiply()
{
}

void EnemyStateMultiply::HandleIA() // spwaned timer <= multiply timer
{
	if (!isMultiplying)
	{
		isMultiplying = true;
		SetSpawnParticles(true);
	}
	else
	{
		spawnTimer -= enemy->App->time->gameDeltaTime;
		multiplyTimer -= enemy->App->time->gameDeltaTime;

		if (!haveSpawned && spawnTimer <= 0)
		{
			math::float3 position = enemy->gameobject->transform->GetGlobalPosition();
			math::float3 playerPos = enemy->enemyController->GetPlayerPosition();
			math::float3 vectorPlayer = math::float3(playerPos.x, position.y, playerPos.z) - position;

			math::float3 sideDirection = vectorPlayer.Cross(math::float3::unitY);
			GameObject* instance1 = enemy->App->scene->Spawn(ENEMY_PREFAB, position + sideDirection.Normalized() * 200.f, Quat(0.f, 0.f, 0.f, 1.f));
			meshInstance1 = enemy->App->scene->FindGameObjectByName("BasicEnemyMesh", instance1)->GetComponent<ComponentRenderer>();
			GameObject* instance2 = enemy->App->scene->Spawn(ENEMY_PREFAB, position - sideDirection.Normalized() * 200.f, Quat(0.f, 0.f, 0.f, 1.f));
			meshInstance2 = enemy->App->scene->FindGameObjectByName("BasicEnemyMesh", instance2)->GetComponent<ComponentRenderer>();

			haveSpawned = true;
		}

		if (haveSpawned)
		{
			meshInstance1->dissolveAmount =
				MAX(0, meshInstance1->dissolveAmount - enemy->App->time->gameDeltaTime * 2.f);
			meshInstance2->dissolveAmount = meshInstance1->dissolveAmount;
			if (multiplyTimer <= 0)
			{
				meshInstance1->dissolveAmount = 0;
				meshInstance2->dissolveAmount = 0;
				SetSpawnParticles(false);
				isMultiplying = false;
				hasMultiplied = true;
			}
		}
	}

	if (hasMultiplied && haveSpawned)
	{
		enemy->currentState = (EnemyState*)enemy->chase;
	}
}

void EnemyStateMultiply::SetSpawnParticles(bool active)
{
	for (size_t i = 0; i < spawnParticleSystems.size(); i++)
	{
		spawnParticleSystems[i]->SetActive(active);
	}
}
