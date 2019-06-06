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

void EnemyStatePatrol::Update()
{
	math::float3 enemyCurrentPosition = enemy->enemyController->GetPosition();
	math::float3 playerCurrentPosition = enemy->enemyController->GetPlayerPosition();
	float distance = enemy->enemyController->GetDistanceTo2D(playerCurrentPosition);

	if (distance < enemy->activationDistance)
		enemy->currentState = (EnemyState*)enemy->getInPosition;
}
