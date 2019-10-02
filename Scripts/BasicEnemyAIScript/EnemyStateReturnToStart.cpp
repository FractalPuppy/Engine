#include "EnemyStateReturnToStart.h"

#include "BasicEnemyAIScript.h"
#include "EnemyControllerScript.h"

#include "GameObject.h"
#include "ComponentTransform.h"

#define START_POS_OFFSET  1.5f

EnemyStateReturnToStart::EnemyStateReturnToStart(BasicEnemyAIScript* AIScript)
{
	enemy = AIScript;
	trigger = "ReturnToStart";
}

EnemyStateReturnToStart::~EnemyStateReturnToStart()
{
}

void EnemyStateReturnToStart::HandleIA()
{
	float distanceToPlayer = enemy->enemyController->GetDistanceToPlayer2D();
	if (distanceToPlayer < enemy->activationDistance)
	{
		enemy->currentState = (EnemyState*)enemy->chase;
	}
	else
	{
		float distanceToStartPosition = enemy->enemyController->GetDistanceTo2D(enemy->startPosition);
		if (distanceToStartPosition < START_POS_OFFSET)
			enemy->currentState = (EnemyState*)enemy->patrol;
	}
}

void EnemyStateReturnToStart::Update()
{
	// Move back to start position
	enemy->enemyController->Move(enemy->chaseSpeed, refreshTime, enemy->startPosition, enemyPath);
}

void EnemyStateReturnToStart::Enter()
{
	//enemy->gameobject->transform->LookAt(enemy->startPosition);
}
