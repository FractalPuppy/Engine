#include "EnemyStateChase.h"

#include "GameObject.h"
#include "ComponentAudioSource.h"
#include "ComponentTransform.h"
#include "Application.h"
#include "ModuleTime.h"

#include "BasicEnemyAIScript.h"
#include "EnemyControllerScript.h"

#include "debugdraw.h"

EnemyStateChase::EnemyStateChase(BasicEnemyAIScript* AIScript)
{
	enemy = AIScript;
	trigger = "Chase";
	inRangeMoan = enemy->gameobject->GetComponent<ComponentAudioSource>();
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

void EnemyStateChase::Enter()
{
	if (inRangeMoan != nullptr)
	{
		inRangeMoan->Play();
	}
}

void EnemyStateChase::Exit()
{
	if (enemy->audioFoot != nullptr)
	{
		enemy->audioFoot->Stop();
	}
	timer = 0.0f;
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
