#include "EnemyStateChase.h"

#include "EnemyControllerScript.h"
#include "FlyingSkullEnemyAIScript.h"

EnemyStateChase::EnemyStateChase(FlyingSkullEnemyAIScript* AIScript)
{
	enemy = AIScript;
}


EnemyStateChase::~EnemyStateChase()
{
}

void EnemyStateChase::HandleIA()
{
	float distanceToPlayer = enemy->enemyController->GetDistanceToPlayer2D();

	if (distanceToPlayer > enemy->disengageDistance)
	{
		enemy->currentState = (EnemyState*)enemy->returnToStart;
	}
	else if (distanceToPlayer < enemy->maxAttackRange)
	{
		enemy->currentState = (EnemyState*)enemy->attack;
	}
}

void EnemyStateChase::Update()
{
	ApproachPlayer();
}

void EnemyStateChase::ApproachPlayer()
{
	math::float3 playerPos = enemy->enemyController->GetPlayerPosition();
	enemy->enemyController->Move(enemy->runSpeed, refreshTime, playerPos, enemyPath);
}
