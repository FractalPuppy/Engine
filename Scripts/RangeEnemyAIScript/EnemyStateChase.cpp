#include "EnemyStateChase.h"

#include "GameObject.h"
#include "ComponentTransform.h"

#include "EnemyControllerScript.h"
#include "RangeEnemyAIScript.h"

#include "debugdraw.h"

EnemyStateChase::EnemyStateChase(RangeEnemyAIScript* AIScript)
{
	enemy = AIScript;
	trigger = "Chase";
}

EnemyStateChase::~EnemyStateChase()
{
}

void EnemyStateChase::HandleIA()
{
	float distanceToPlayer = enemy->enemyController->GetDistanceToPlayer2D();
	
	if (distanceToPlayer > enemy->disengageDistance)
	{
		// Too far: Return to start position
		enemy->currentState = (EnemyState*)enemy->returnToStart;
	}
	//else if (distanceToPlayer < enemy->minAttackDistance)
	//{
	//	enemy->currentState = (EnemyState*)enemy->flee;
	//}
	else if(distanceToPlayer < enemy->maxAttackDistance /*&& distanceToPlayer > enemy->minAttackDistance*/)
	{
		// Player in range: change to attack
		enemy->currentState = (EnemyState*)enemy->attack;
	}
}

void EnemyStateChase::Update()
{
	AproachPlayer();
}

void EnemyStateChase::AproachPlayer()
{
	math::float3 playerPos = enemy->enemyController->GetPlayerPosition();
	enemy->enemyController->Move(enemy->runSpeed, refreshTime, playerPos, enemyPath);

	if (enemy->drawDebug)
	{
		dd::point(playerPos, dd::colors::Purple, 10.0f);
		dd::line(enemy->enemyController->GetPosition(), playerPos, dd::colors::Purple);
	}
}


