#include "EnemyStatePatrol.h"

#include "RangeEnemyAIScript.h"
#include "EnemyControllerScript.h"

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

void EnemyStatePatrol::Update()
{
	//Patrol movement should be implemented here
}
