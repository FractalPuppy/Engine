#include "Application.h"
#include "ModuleTime.h"
#include "ModuleScene.h"

#include "GameObject.h"
#include "ComponentTransform.h"

#include "BossStateThirdRight.h"

#include "BossBehaviourScript.h"

#include "EnemyControllerScript/EnemyControllerScript.h"
#include "ComponentAnimation.h"

#define THIRDAOE "ThirdPhaseAOEPrefab"

BossStateThirdRight::BossStateThirdRight(BossBehaviourScript* AIBoss)
{
	boss = AIBoss;
	trigger = "ThirdRight";
}


BossStateThirdRight::~BossStateThirdRight()
{
}

void BossStateThirdRight::HandleIA()
{
	if (duration < timer)
	{
		boss->currentState = (BossState*)boss->thirdIdle;
	}
}

void BossStateThirdRight::Update()
{
	if (!prefabSpawned)
	{
		math::float3 direction = boss->playerPosition - boss->currentPosition;
		direction.y = 0;
		direction.Normalize();
		math::float3 newPosition = boss->currentPosition + direction * 940.0f;
		newPosition.y = boss->playerPosition.y;
		boss->App->scene->Spawn(THIRDAOE, newPosition, boss->currentRotation);
		prefabSpawned = true;
	}
}

void BossStateThirdRight::Enter()
{
	boss->enemyController->anim->SendTriggerToStateMachine(trigger.c_str());
	duration = boss->enemyController->anim->GetDurationFromClip();
	prefabSpawned = false;
}

void BossStateThirdRight::Exit()
{
}

math::float3 BossStateThirdRight::GetFistPosition()
{
	math::float3 result = boss->rightFist->transform->position;
	return math::float3(result.x, boss->playerPosition.y + 20.f, result.z);
}