#include "EnemyStateFlee.h"

#include "BasicEnemyAIScript.h"
#include "EnemyControllerScript.h"

#include "GameObject.h"
#include "ComponentTransform.h"

EnemyStateFlee::EnemyStateFlee(BasicEnemyAIScript* AIScript)
{
	enemy = AIScript;
	trigger = "Flee";
}


EnemyStateFlee::~EnemyStateFlee()
{
}

void EnemyStateFlee::Enter()
{
	duration = 1.5f;
	enemy->scared = false;
}

void EnemyStateFlee::HandleIA()
{
	if (timer > duration)
	{
		enemy->currentState = (EnemyState*)enemy->chase;
	}
}

void EnemyStateFlee::Update()
{
	MoveAwayFromPlayer();
}

void EnemyStateFlee::MoveAwayFromPlayer()
{
	float distanceToPlayer = enemy->enemyController->GetDistanceToPlayer2D();

	math::float3 playerPos = enemy->enemyController->GetPlayerPosition();
	math::float3 enemyPos = enemy->enemyController->GetPosition();

	// Get Euclidean Vector (Vector Director) 
	float x = enemyPos.x - playerPos.x;
	float z = enemyPos.z - playerPos.z;

	// Normalize
	x /= distanceToPlayer;
	z /= distanceToPlayer;

	// Get the new point which is at maxAttackDistance from player position
	math::float3 newPosition;
	newPosition.x = ((float)playerPos.x + x * (1000.0f));
	newPosition.y = enemyPos.y;
	newPosition.z = ((float)playerPos.z + z * (1000.0f));

	//enemy->gameobject->transform->LookAt(newPosition);

	// Move to that point
	enemy->enemyController->Move(enemy->chaseSpeed, refreshTime, newPosition, enemyPath);
}