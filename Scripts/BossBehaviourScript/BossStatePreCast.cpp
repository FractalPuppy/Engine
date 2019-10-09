#include "Application.h"
#include "ModuleScene.h"
#include "ModuleTime.h"

#include "GameObject.h"

#include "BossStatePreCast.h"

#include "BossBehaviourScript.h"
#include "EnemyControllerScript/EnemyControllerScript.h"
#include "ComponentAnimation.h"

BossStatePreCast::BossStatePreCast(BossBehaviourScript* AIBoss)
{
	boss = AIBoss;
	trigger = "PrecastNL";
}


BossStatePreCast::~BossStatePreCast()
{
}

void BossStatePreCast::HandleIA()
{
	if (timer > duration)
	{
		boss->currentState = (BossState*)boss->cast;
	}
}

void BossStatePreCast::Update()
{
	boss->enemyController->LookAt2D(boss->playerPosition);

	if (!orbsSet)
	{
		if (timerOrbs/duration > boss->percOrbs)
		{
			boss->leftHandBall->SetActive(true);
			boss->rightHandBall->SetActive(true);
			boss->leftHandParticles->SetActive(true);
			boss->rightHandParticles->SetActive(true);
			orbsSet = true;
		}
		else
		{
			timerOrbs += boss->App->time->gameDeltaTime;
		}
	}
}

void BossStatePreCast::Enter()
{
	boss->anim->SendTriggerToStateMachine(trigger.c_str());
	duration = boss->anim->GetDurationFromClip();
}

void BossStatePreCast::Exit()
{
	timerOrbs = 0.0f;
	orbsSet = false;
}
