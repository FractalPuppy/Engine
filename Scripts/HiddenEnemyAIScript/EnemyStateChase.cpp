#include "EnemyStateChase.h"

#include "GameObject.h"
#include "ComponentTransform.h"

#include "HiddenEnemyAIScript.h"
#include "EnemyControllerScript.h"

#include "debugdraw.h"

EnemyStateChase::EnemyStateChase(HiddenEnemyAIScript* AIScript)
{
	enemy = AIScript;
	trigger = "Chase";
}

EnemyStateChase::~EnemyStateChase()
{

}

void EnemyStateChase::HandleIA()
{
	float distance = enemy->enemyController->GetDistanceToPlayer2D();
	if (distance < enemy->attackRange)
	{
		enemy->currentState = (EnemyState*)enemy->attack;
	}
	else if (distance > enemy->returnDistance)
	{
		enemy->currentState = (EnemyState*)enemy->returnToStart;
	}
}

void EnemyStateChase::Update()
{
	// Move towards the player
	enemy->enemyController->Move(enemy->chaseSpeed, refreshTime, enemy->enemyController->GetPlayerPosition(), enemyPath);

	if (enemy->drawDebug)
	{
		math::float3 playerPos = enemy->enemyController->GetPlayerPosition();
		dd::point(playerPos, dd::colors::Purple, 10.0f);
		dd::line(enemy->enemyController->GetPosition(), playerPos, dd::colors::Purple);
	}
}
