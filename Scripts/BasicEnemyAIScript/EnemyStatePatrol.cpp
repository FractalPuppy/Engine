#include "EnemyStatePatrol.h"

#include "BasicEnemyAIScript.h"
#include "EnemyControllerScript.h"

#include "Math/float3.h"

EnemyStatePatrol::EnemyStatePatrol(BasicEnemyAIScript* AIScript)
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
		enemy->currentState = (EnemyState*)enemy->chase;
}
