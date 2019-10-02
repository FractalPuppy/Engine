#include "EnemyStatePatrol.h"

#include "EnemyControllerScript.h"
#include "FlyingSkullEnemyAIScript.h"

EnemyStatePatrol::EnemyStatePatrol(FlyingSkullEnemyAIScript* AIScript)
{
	enemy = AIScript;
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

}