#include "BossStateThirdIdle.h"

#include "BossBehaviourScript.h"

#include "EnemyControllerScript/EnemyControllerScript.h"
#include "ComponentAnimation.h"

BossStateThirdIdle::BossStateThirdIdle(BossBehaviourScript* AIBoss)
{
	boss = AIBoss;
	trigger = "ThirdIdle";
}


BossStateThirdIdle::~BossStateThirdIdle()
{
}

void BossStateThirdIdle::HandleIA()
{
	if (duration < timer)
	{
		switch (nextFist)
		{
		case Fist::Left:
			boss->currentState = (BossState*)boss->thirdLeft;
			break;
		case Fist::Right:
			boss->currentState = (BossState*)boss->thirdRight;
			break;
		}
	}
}

void BossStateThirdIdle::Update()
{
	//Should we do a look at?
	boss->enemyController->LookAt2D(boss->playerPosition);
}

void BossStateThirdIdle::Enter()
{
	duration = 2.0f + (std::rand() % 100) / 100.f;
	nextFist = ChooseNextFist();
	boss->enemyController->anim->SendTriggerToStateMachine(trigger.c_str());
}

void BossStateThirdIdle::Exit()
{

}

Fist BossStateThirdIdle::ChooseNextFist()
{
	while (nextFist == lastFist && nextFist == secondLastFist)
	{
		int randomNumb = rand() % 2;

		switch (randomNumb)
		{
		case 0:
			nextFist = Fist::Right;
			break;
		case 1:
			nextFist = Fist::Left;
			break;
		}
	}

	secondLastFist = lastFist;
	lastFist = nextFist;

	return nextFist;
}
