#include "EnemyStateChase.h"

#include "GameObject.h"
#include "ComponentAudioSource.h"
#include "ComponentTransform.h"
#include "Application.h"
#include "ModuleTime.h"
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
	timer += enemy->App->time->gameDeltaTime;
	if (timer >= walkTimer && enemy->audioFoot != nullptr)
	{
		timer = 0.0f;
		enemy->audioFoot->Play();
		float offset = enemy->randomOffset(0.4) - 0.2;
		enemy->audioFoot->SetPitch(1.0 + offset);
	}
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


