#include "Application.h"
#include "ModuleTime.h"
#include "ModuleScene.h"

#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentAnimation.h"
#include "ComponentCamera.h"

#include "BossStateSummonArmy.h"

#include "BossBehaviourScript.h"
#include "CameraController/CameraController.h"
#include "BasicEnemyAIScript/BasicEnemyAIScript.h"
#include "EnemyControllerScript/EnemyControllerScript.h"

#define BASICSUMMON "BasicBossSummon"

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
	switch (animState)
	{
	case animationState::None:
		animState = animationState::Precast;
		boss->anim->SendTriggerToStateMachine("PrecastNL");
		animDuration = boss->anim->GetDurationFromClip();
		break;
	case animationState::Precast:
		if (animTimer > animDuration)
		{
			boss->anim->SendTriggerToStateMachine("Cast");
			animDuration = boss->anim->GetDurationFromClip();
			animState = animationState::Cast;
			animTimer = 0.0f;
		}
		else
		{
			animTimer += boss->App->time->gameDeltaTime;

			//we have to activate orbs somewhere here
		}
		break;
	case animationState::Cast:
		if (animTimer > animDuration)
		{
			boss->anim->SendTriggerToStateMachine("Idle");
			animState = animationState::Finished;
			animTimer = 0.0f;
		}
		else
		{
			animTimer += boss->App->time->gameDeltaTime;
			//we have to deactivate orbs somewhere here also set casted to true
			casted = true;
		}
		break;
	case animationState::Finished:
		break;
	}


	if (casted)
	{
		timerSkeletons += boss->App->time->gameDeltaTime;
		if (!firstSummon || timerSkeletons > boss->timerBetweenSummonsSummonPhase)
		{
			//SPAWN one enemy at a random spawn location
			math::float3 spawnLocation = boss->ChooseRandomSpawn();

			GameObject* firstSkeleton = boss->App->scene->Spawn(BASICSUMMON, spawnLocation, math::Quat::identity);

			//We need this so they agro automatically
			firstSkeleton->GetComponent<BasicEnemyAIScript>()->activationDistance = 9000.0f;
			firstSkeleton->GetComponent<BasicEnemyAIScript>()->returnDistance = 9000.0f;

			enemiesSpawned += 1;

			firstSkeleton->transform->LookAtLocal(boss->playerPosition);

			timerSkeletons = 0.0f;
			firstSummon = true;
		}

		LerpFogColor();
	}
	
}

void BossStateSummonArmy::Enter()
{
	initialColor = boss->compCamera->fogColor;
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

void BossStateSummonArmy::LerpFogColor()
{
	if (colorTimer < boss->fogLerpDuration)
	{
		float lambda = colorTimer / boss->fogLerpDuration;
	
		boss->compCamera->SetFogColor(boss->InterpolateFloat3(initialColor, boss->fogFinalColor, lambda));

		colorTimer += boss->App->time->gameDeltaTime;
	}


}
