#include "BossStateThirdDeath.h"

#include "BossBehaviourScript.h"

#include "EnemyControllerScript/EnemyControllerScript.h"
#include "GameLoop/GameLoop.h"
#include "ComponentAnimation.h"

BossStateThirdDeath::BossStateThirdDeath(BossBehaviourScript* AIBoss)
{
	boss = AIBoss;
	trigger = "ThirdDeath";
}


BossStateThirdDeath::~BossStateThirdDeath()
{
}

void BossStateThirdDeath::HandleIA()
{

}

void BossStateThirdDeath::Update()
{
	//superdead, allegedly
	if (timer > 4.0f)
	{
		boss->gLoop->bossDeath = true;
	}
}

void BossStateThirdDeath::Enter()
{
	boss->enemyController->anim->SendTriggerToStateMachine(trigger.c_str());
	boss->ResetVariables();
}

void BossStateThirdDeath::Exit()
{
}
