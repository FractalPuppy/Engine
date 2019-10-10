#include "Application.h"
#include "ModuleTime.h"
#include "ModuleScene.h"

#include "GameObject.h"
#include "ComponentRenderer.h"
#include "ComponentTransform.h"
#include "ComponentAnimation.h"

#include "BossStateDeath.h"

#include "BossBehaviourScript.h"
#include "EnemyControllerScript/EnemyControllerScript.h"
#include "GameLoop/GameLoop.h"

BossStateDeath::BossStateDeath(BossBehaviourScript* AIBoss)
{
	boss = AIBoss;
	trigger = "Kneel";
}


BossStateDeath::~BossStateDeath()
{
}

void BossStateDeath::Update()
{
	ComponentRenderer* bossRenderer = boss->enemyController->GetMainRenderer();

	if (timer < 3.0f)
	{
		bossRenderer->dissolveAmount += boss->App->time->gameDeltaTime;
	}
	else
	{
		//superdead, allegedly
		boss->gLoop->bossDeath = true;
	}
}

void BossStateDeath::Enter()
{
	boss->anim->SendTriggerToStateMachine(trigger.c_str());
}
