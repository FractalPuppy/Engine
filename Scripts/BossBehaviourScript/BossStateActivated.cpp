#include "Application.h"
#include "ModuleTime.h"

#include "BossStateActivated.h"
#include "BossBehaviourScript.h"
#include "EnemyControllerScript/EnemyControllerScript.h"
#include "CameraController/CameraController.h"

#include "GameObject.h"
#include "ComponentTransform.h"

#include "Math/float3.h"

BossStateActivated::BossStateActivated(BossBehaviourScript* AIBoss)
{
	boss = AIBoss;
}


BossStateActivated::~BossStateActivated()
{
}

void BossStateActivated::HandleIA()
{
	if (timer > boss->activationTime)
	{
		boss->bossPhase = BossPhase::First;
		boss->currentLocation = TPlocations::None;
		boss->currentState = (BossState*)boss->idle;
	}
}

void BossStateActivated::Update()
{
	//Here the boss comes down from the throne and when it's finished the fight starts
	if (timer < timeToHighPoint)
	{
		//Go high
		math::float3 newPosition = boss->enemyController->GetPosition() + directionToHigh * speedUntilHigh * boss->App->time->gameDeltaTime;
		boss->enemyController->SetPosition(newPosition);
	}
	else if (timer > (timeFloating + timeToHighPoint))
	{
		//Descend
		math::float3 newPosition = boss->enemyController->GetPosition() + directionToGround * speedDescend * boss->App->time->gameDeltaTime;
		boss->enemyController->SetPosition(newPosition);
	}

}


void BossStateActivated::Enter()
{
	timeToHighPoint = boss->activationTime * percToHigh;
	timeFloating = boss->activationTime * percFloating;
	timeToGround = boss->activationTime * (1 - percToHigh - percFloating);

	directionToHigh = boss->highPointFirstCS - boss->startingPoint;
	directionToHigh.Normalize();
	speedUntilHigh = (boss->startingPoint.Distance(boss->highPointFirstCS)) / timeToHighPoint;
	directionToGround = boss->pointStartFight - boss->highPointFirstCS;
	directionToGround.Normalize();
	speedDescend = (boss->highPointFirstCS.Distance(boss->pointStartFight)) / timeToGround;
}