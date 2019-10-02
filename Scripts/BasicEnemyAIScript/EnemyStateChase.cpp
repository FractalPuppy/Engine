#include "EnemyStateChase.h"

#include "GameObject.h"
#include "ComponentTransform.h"

#include "BasicEnemyAIScript.h"
#include "EnemyControllerScript.h"

#include "debugdraw.h"

EnemyStateChase::EnemyStateChase(BasicEnemyAIScript* AIScript)
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
	if (enemy->scared)
	{
		enemy->currentState = (EnemyState*)enemy->flee;
	}
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
	//if player has moved since last time we checked, make a new move request
	float diffX = abs(positionGoingTowards.x - enemy->enemyController->GetPlayerPosition().x);
	float diffZ = abs(positionGoingTowards.z - enemy->enemyController->GetPlayerPosition().z);
	if (diffX > 50.f || diffZ > 50.f)
	{
		// Move towards the player
		positionGoingTowards = enemy->enemyController->GetPlayerPosition();
		enemy->enemyController->Move(enemy->chaseSpeed, refreshTime, positionGoingTowards, enemyPath);
		//enemy->gameobject->transform->LookAt(positionGoingTowards);
	}

	if (enemy->drawDebug)
	{
		math::float3 playerPos = enemy->enemyController->GetPlayerPosition();
		dd::point(playerPos, dd::colors::Purple, 10.0f);
		dd::line(enemy->enemyController->GetPosition(), playerPos, dd::colors::Purple);
	}
}
