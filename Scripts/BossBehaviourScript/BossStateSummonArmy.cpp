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
}

void BossStateSummonArmy::HandleIA()
{
	if (AllEnemiesAppeared())
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
			//SPAWN one enemy at a random spawn location
			math::float3 spawnLocation = boss->ChooseRandomSpawn();

			GameObject* firstSkeleton = boss->App->scene->Spawn(BASICSUMMON, spawnLocation, math::Quat::identity);

			//We need this so they agro automatically
			firstSkeleton->GetComponent<BasicEnemyAIScript>()->activationDistance = 9000.0f;
			firstSkeleton->GetComponent<BasicEnemyAIScript>()->returnDistance = 9000.0f;

			enemiesSpawned += 1;

			firstSkeleton->transform->LookAt(boss->playerPosition);

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

bool BossStateSummonArmy::AllEnemiesAppeared()
{
	bool ret = false;

	if (enemiesSpawned >= boss->summonSkeletonsNumber)
	{
		ret = true;
	}
	return ret;
}
