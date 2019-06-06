#include "EnemyStateGetInPosition.h"

#include "GameObject.h"
#include "ComponentTransform.h"

#include "EnemyControllerScript.h"
#include "RangeEnemyAIScript.h"

EnemyStateGetInPosition::EnemyStateGetInPosition(RangeEnemyAIScript* AIScript)
{
	enemy = AIScript;
	trigger = "GetInPosition";
}

EnemyStateGetInPosition::~EnemyStateGetInPosition()
{
}

void EnemyStateGetInPosition::Update()
{
	// Look at player and move towards
	//enemy->gameobject->transform->LookAt(enemy->enemyController->GetPlayerPosition());

	float distanceToPlayer = enemy->enemyController->GetDistanceToPlayer2D();

	if (distanceToPlayer > enemy->maxAttackDistance)
	{
		//enemy->enemyController->MoveTowards(enemy->runSpeed);
		enemy->enemyController->Move(enemy->runSpeed, refreshTime, enemy->enemyController->GetPlayerPosition(), enemyPath);
	}
	else
	{
		math::float3 direction = enemy->gameobject->transform->front;
		direction.y *= -1.0f;
		enemy->enemyController->Move(enemy->runSpeed, direction);
	}

	// Check distance to player
	if (distanceToPlayer < enemy->maxAttackDistance && distanceToPlayer > enemy->minAttackDistance)
	{
		// Player in range, change to attack
		enemy->currentState = (EnemyState*)enemy->attack;
	}
	else if(distanceToPlayer > enemy->disengageDistance)
	{
		// Return to start position
		enemy->currentState = (EnemyState*)enemy->returnToStart;
	}
}
