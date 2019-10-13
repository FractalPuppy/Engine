#include "EnemyStateFlee.h"

#include "EnemyControllerScript.h"
#include "RangeEnemyAIScript.h"

#include "Application.h"
#include "ModuleNavigation.h"

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
	else if (skipFlee)
	{
		enemy->currentState = (EnemyState*)enemy->attack;
	}
}

void EnemyStateFlee::Update()
{
	MoveAwayFromPlayer();
}

void EnemyStateFlee::Enter()
{
	duration = 2.0f;
	FindFleeDirection();
	enemy->enemyController->Move(enemy->runSpeed, refreshTime, fleeDestiny, enemyPath);
}

void EnemyStateFlee::Exit()
{
	enemy->enemyController->LookAt2D(enemy->enemyController->GetPlayerPosition());
	changedTimes = 0;
	skipFlee = false;
}

void EnemyStateFlee::FindFleeDirection()
{
	float distanceToPlayer = enemy->enemyController->GetDistanceToPlayer2D();
	playerPos = enemy->enemyController->GetPlayerPosition();
	enemyPos = enemy->enemyController->GetPosition();

	fleeDir = (enemyPos - playerPos).Normalized();
	fleeDestiny = playerPos + fleeDir * enemy->maxAttackDistance;
}

void EnemyStateFlee::MoveAwayFromPlayer()
{
	// Move to that point
	if (enemy->enemyController->GetPlayerPosition()
		.Distance(enemy->enemyController->GetPosition()) > 0.9f * enemy->maxAttackDistance)
	{
		skipFlee = true;
		return;
	}
	if (enemy->enemyController->IsStuck())
	{
		ChangeDirection();
	}

	if (enemy->drawDebug)
	{
		dd::point(fleeDestiny, dd::colors::Purple, 10.0f);
		dd::line(enemyPos, fleeDestiny, dd::colors::Purple);
	}
}

void EnemyStateFlee::ChangeDirection()
{
	math::Quat quat;
	math::LCG rand;

	if (changedTimes < 2)
	{
		quat = Quat::RotateY(math::DegToRad(rand.Float() * 40 - 20));
	}

	std::vector<math::float3> path;
	do
	{
		fleeDir = quat.Mul(fleeDir);
		fleeDestiny = playerPos + fleeDir * enemy->maxAttackDistance;
		changedTimes++;
	} while (changedTimes < 3 && !enemy->App->navigation->FindPath(enemy->enemyController->GetPosition(), fleeDestiny, path));

	if (changedTimes > 2)
	{
		skipFlee = true;
	}
	else
	{
		enemy->enemyController->Move(enemy->runSpeed, refreshTime, fleeDestiny, enemyPath);
	}
}
