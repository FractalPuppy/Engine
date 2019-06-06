#include "EnemyStateWait.h"

#include "HiddenEnemyAIScript.h"
#include "EnemyControllerScript.h"

#include "Math/float3.h"

EnemyStateWait::EnemyStateWait(HiddenEnemyAIScript* AIScript)
{
	enemy = AIScript;
	trigger = "Wait";
}

EnemyStateWait::~EnemyStateWait()
{
}

void EnemyStateWait::Update()
{
	math::float3 enemyCurrentPosition = enemy->enemyController->GetPosition();
	math::float3 playerCurrentPosition = enemy->enemyController->GetPlayerPosition();
	float distance = enemy->enemyController->GetDistanceTo2D(playerCurrentPosition);

	if (distance < enemy->activationDistance)
		enemy->currentState = (EnemyState*)enemy->showUp;
}
