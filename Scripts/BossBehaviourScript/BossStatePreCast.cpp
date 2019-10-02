#include "BossStatePreCast.h"

#include "BossBehaviourScript.h"
#include "EnemyControllerScript/EnemyControllerScript.h"

BossStatePreCast::BossStatePreCast(BossBehaviourScript* AIBoss)
{
	boss = AIBoss;
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
}

void BossStatePreCast::Enter()
{
	
	duration = 4.0f;
}
