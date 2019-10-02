#include "BossStateIdle.h"

#include "BossBehaviourScript.h"
#include "EnemyControllerScript/EnemyControllerScript.h"

BossStateIdle::BossStateIdle(BossBehaviourScript* AIBoss)
{
	boss = AIBoss;
}


BossStateIdle::~BossStateIdle()
{
}

void BossStateIdle::HandleIA()
{
	if (timer > (duration + baseDuration))
	{
		boss->currentState = (BossState*)boss->precast;
	}
}

void BossStateIdle::Update()
{
	boss->enemyController->LookAt2D(boss->playerPosition);
}

void BossStateIdle::Enter()
{
	duration = (std::rand() % 100) / 100.f;
}
