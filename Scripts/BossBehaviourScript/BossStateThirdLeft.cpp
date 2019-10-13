#include "Application.h"
#include "ModuleTime.h"
#include "ModuleScene.h"

#include "GameObject.h"
#include "ComponentTransform.h"

#include "BossStateThirdLeft.h"

#include "BossBehaviourScript.h"

#include "EnemyControllerScript/EnemyControllerScript.h"
#include "ComponentAnimation.h"

#define THIRDAOE "ThirdPhaseAOEPrefab"

#include "Math/float3.h"

BossStateThirdLeft::BossStateThirdLeft(BossBehaviourScript* AIBoss)
{
	boss = AIBoss;
	trigger = "ThirdLeft";
}


BossStateThirdLeft::~BossStateThirdLeft()
{
}

void BossStateThirdLeft::HandleIA()
{
	if (duration < timer)
	{
		boss->currentState = (BossState*)boss->thirdIdle;
	}
}

void BossStateThirdLeft::Update()
{
	//Need to create the hitbox
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

void BossStateThirdLeft::Enter()
{
	boss->enemyController->anim->SendTriggerToStateMachine(trigger.c_str());
	duration = boss->enemyController->anim->GetDurationFromClip();
	prefabSpawned = false;
}

void BossStateThirdLeft::Exit()
{
}

math::float3 BossStateThirdLeft::GetFistPosition()
{
	math::float3 result = boss->leftFist->transform->position;

	return math::float3(result.x, boss->playerPosition.y + 20.f, result.z);
}