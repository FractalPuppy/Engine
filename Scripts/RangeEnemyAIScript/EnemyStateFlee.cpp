#include "EnemyStateFlee.h"

#include "EnemyControllerScript.h"
#include "RangeEnemyAIScript.h"

#include "GameObject.h"
#include "ComponentTransform.h"

#include "debugdraw.h"

EnemyStateFlee::EnemyStateFlee(RangeEnemyAIScript* AIScript)
{
	enemy = AIScript;
	trigger = "Flee";
}


EnemyStateFlee::~EnemyStateFlee()
{
}

void EnemyStateFlee::HandleIA()
{
	
	if (timer > duration)
	{
		float distanceToPlayer = enemy->enemyController->GetDistanceToPlayer2D();

		if(distanceToPlayer > enemy->maxAttackDistance)
		{
			if (distanceToPlayer > enemy->disengageDistance)
			{
				enemy->currentState = (EnemyState*)enemy->returnToStart;
			}
			else
			{
				enemy->currentState = (EnemyState*)enemy->chase;
			}
		}
		else
		{
			enemy->currentState = (EnemyState*)enemy->attack;
		}
	}
}

void EnemyStateFlee::Update()
{
	MoveAwayFromPlayer();
}

void EnemyStateFlee::Enter()
{
	duration = 2.0f;
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
	newPosition.x = ((float)playerPos.x + x * (enemy->maxAttackDistance));
	newPosition.y = enemyPos.y;
	newPosition.z = ((float)playerPos.z + z * (enemy->maxAttackDistance));

	// Move to that point
	enemy->enemyController->Move(enemy->runSpeed, refreshTime, newPosition, enemyPath);
	enemy->gameobject->transform->LookAt(newPosition);

	if (enemy->drawDebug)
	{
		dd::point(newPosition, dd::colors::Purple, 10.0f);
		dd::line(enemyPos, newPosition, dd::colors::Purple);
	}
}