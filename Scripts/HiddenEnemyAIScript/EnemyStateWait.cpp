#include "EnemyStateWait.h"

#include "HiddenEnemyAIScript.h"
#include "EnemyControllerScript.h"
#include "ComponentRenderer.h"

#include "Math/float3.h"

EnemyStateWait::EnemyStateWait(HiddenEnemyAIScript* AIScript)
{
	enemy = AIScript;
	trigger = "Wait";
}

EnemyStateWait::~EnemyStateWait()
{
}

void EnemyStateWait::HandleIA()
{
	float distanceToPlayer = enemy->enemyController->GetDistanceToPlayer2D();

	if (distanceToPlayer < enemy->activationDistance)
		enemy->currentState = (EnemyState*)enemy->showUp;
}

void EnemyStateWait::Update()
{

}
