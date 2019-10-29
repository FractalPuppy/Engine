#include "EnemyStatePatrol.h"

#include "RangeEnemyAIScript.h"
#include "EnemyControllerScript.h"
#include "GameObject.h"
#include "ComponentTransform.h"

#include "Math/float3.h"

EnemyStatePatrol::EnemyStatePatrol(RangeEnemyAIScript* AIScript)
{
	enemy = AIScript;
	trigger = "Patrol";
}

EnemyStatePatrol::~EnemyStatePatrol()
{
}

void EnemyStatePatrol::HandleIA()
{
	float distance = enemy->enemyController->GetDistanceToPlayer2D();
	if (distance < enemy->activationDistance)
		enemy->currentState = (EnemyState*)enemy->chase;
}

void EnemyStatePatrol::Enter()
{
	//startPos = enemy->enemyController->GetPosition();
	//endPos = startPos + enemy->gameobject->transform->right * patrolDistance;
	//destiny = endPos;
}

void EnemyStatePatrol::Update() //TODO: We should modify the animations SM
{
	//enemy->enemyController->Move(enemy->runSpeed, refreshTime, destiny, enemyPath);
	//if (enemy->gameobject->transform->position.Equals(destiny, 20.f))
	//{
	//	if (destiny.Equals(endPos))
	//	{
	//		destiny = startPos;
	//	}
	//	else
	//	{
	//		destiny = endPos;
	//	}
	//}
}
