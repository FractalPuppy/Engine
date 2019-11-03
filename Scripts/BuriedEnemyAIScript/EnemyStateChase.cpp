#include "EnemyStateChase.h"

#include "BuriedEnemyAIScript.h"
#include "EnemyControllerScript.h"
#include "Application.h"
#include "ModuleTime.h"
#include "ComponentAudioSource.h"

EnemyStateChase::EnemyStateChase(BuriedEnemyAIScript* AIScript)
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
		enemy->currentState = (EnemyState*)enemy->returnToStart;
	}
	else if (distanceToPlayer < enemy->maxAttackRange && distanceToPlayer > enemy->minAttackRange)
	{
		enemy->currentState = (EnemyState*)enemy->attack;
		enemy->enemyController->Stop();
	}
	else if (distanceToPlayer < enemy->minAttackRange && enemy->teleportAvailable)
	{
		enemy->currentState = (EnemyState*)enemy->relocate;
	}
	else if (distanceToPlayer < enemy->minAttackRange && !enemy->teleportAvailable)
	{
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
	// Move towards the player
	enemy->enemyController->Move(enemy->runSpeed, refreshTime, enemy->enemyController->GetPlayerPosition(), enemyPath);
}

void EnemyStateChase::Exit()
{
	if (enemy->audioFoot != nullptr)
	{
		enemy->audioFoot->Stop();
	}
}