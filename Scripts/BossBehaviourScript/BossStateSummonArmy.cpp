#include "Application.h"
#include "ModuleTime.h"
#include "ModuleScene.h"

#include "GameObject.h"
#include "ComponentTransform.h"

#include "BossStateSummonArmy.h"

#include "BossBehaviourScript.h"
#include "CameraController/CameraController.h"
#include "BasicEnemyAIScript/BasicEnemyAIScript.h"
#include "EnemyControllerScript/EnemyControllerScript.h"

#define BASICSUMMON "BasicEnemy"

BossStateSummonArmy::BossStateSummonArmy(BossBehaviourScript* AIBoss)
{
	boss = AIBoss;
}


BossStateSummonArmy::~BossStateSummonArmy()
{
	enemies.clear();
}

void BossStateSummonArmy::HandleIA()
{
	if (AllEnemiesDead())
	{
		boss->currentState = (BossState*)boss->activated;
	}
}

void BossStateSummonArmy::Update()
{
	if (timer > downTime)
	{
		timerSkeletons += boss->App->time->gameDeltaTime;
		if (timerSkeletons > boss->timerBetweenSummons)
		{
			//SPAWN two guys
			math::float3 spawnLocation = ChooseFurthestSpawn();
			math::float3 directionToPlayer = boss->playerPosition - spawnLocation;
			math::float3 sideVector = directionToPlayer.Cross(math::float3::unitY);
			sideVector.Normalize();


			GameObject* firstSkeleton = boss->App->scene->Spawn(BASICSUMMON, spawnLocation + sideVector * 200.0f, math::Quat::identity);
			GameObject* secondSkeleton = boss->App->scene->Spawn(BASICSUMMON, spawnLocation - sideVector * 200.0f, math::Quat::identity);

			firstSkeleton->GetComponent<BasicEnemyAIScript>()->activationDistance = 9000.0f;
			firstSkeleton->GetComponent<BasicEnemyAIScript>()->returnDistance = 9000.0f;
			secondSkeleton->GetComponent<BasicEnemyAIScript>()->activationDistance = 9000.0f;
			secondSkeleton->GetComponent<BasicEnemyAIScript>()->returnDistance = 9000.0f;


			enemiesSpawned += 2;

			enemies.push_back(firstSkeleton->GetComponent<EnemyControllerScript>());
			enemies.push_back(secondSkeleton->GetComponent<EnemyControllerScript>());

			firstSkeleton->transform->LookAt(boss->playerPosition);
			secondSkeleton->transform->LookAt(boss->playerPosition);

			timerSkeletons = 0.0f;
		}
	}
	
}

void BossStateSummonArmy::Enter()
{
}

void BossStateSummonArmy::Exit()
{
}

bool BossStateSummonArmy::AllEnemiesDead()
{
	bool ret = false;

	if (enemiesSpawned >= boss->summonSkeletonsNumber)
	{
		for (auto enemy : enemies)
		{
			if (!enemy->isDead)
			{
				ret = false;
				break;
			}
		}
		ret = true;
	}
	return ret;
}

math::float3 BossStateSummonArmy::ChooseFurthestSpawn()
{
	float distanceFirst = boss->playerPosition.DistanceSq(boss->firstSpawnLocation);
	float distanceSecond = boss->playerPosition.DistanceSq(boss->secondSpawnLocation);
	if (distanceFirst > distanceSecond)
	{
		return boss->firstSpawnLocation;
	}
	else
	{
		return boss->secondSpawnLocation;
	}

}
