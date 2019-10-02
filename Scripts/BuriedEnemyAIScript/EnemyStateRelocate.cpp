#include "EnemyStateRelocate.h"

#include "BuriedEnemyAIScript.h"
#include "EnemyControllerScript.h"

#include "Application.h"
#include "ModuleTime.h"
#include "ModuleNavigation.h"

#include "GameObject.h"
#include "ComponentRenderer.h"
#include "ComponentBoxTrigger.h"
#include "ComponentTransform.h"

#include "Math/float3.h"
#include "Math/MathFunc.h"
#include "Geometry/Frustum.h"

EnemyStateRelocate::EnemyStateRelocate(BuriedEnemyAIScript* AIScript)
{
	enemy = AIScript;
	trigger = "Relocate";
}


EnemyStateRelocate::~EnemyStateRelocate()
{
}

void EnemyStateRelocate::HandleIA()
{
	if (timer > duration)
	{
		enemy->currentState = (EnemyState*)enemy->showUp;
		finished = true;
		ChangePosition();
	}
}

void EnemyStateRelocate::Update()
{

}

void EnemyStateRelocate::ChangePosition()
{
	//Here we find a point far from the player, preferably in the other direction
	math::float3 initialPos = enemy->enemyController->GetPosition();
	math::float3 playerPos = enemy->enemyController->GetPlayerPosition();

	math::float3 direction = playerPos - initialPos;
	direction.Normalize();

	math::float3 newPos = playerPos + direction * enemy->teleportDistance;
	

	if (enemy->App->navigation->FindClosestPoint2D(newPos))
	{
		enemy->enemyController->SetPosition(newPos);		
		enemy->StartTeleportCD();
		enemy->enemyController->Move(enemy->runSpeed, refreshTime, enemy->enemyController->GetPosition(), enemyPath);
	}
}

void EnemyStateRelocate::Exit()
{
	enemy->enemyController->GetMainRenderer()->Enable(false);
	enemy->dustParticlesGO->SetActive(false);

	// Set candle on the ground
	float candleZPos = enemy->candleGO->transform->position.z;
	if (candleZPos > candleZPos - enemy->candleOffset)
		enemy->candleGO->transform->position.z -= enemy->candleOffset;
}

void EnemyStateRelocate::Enter()
{
	enemy->enemyController->hpBoxTrigger->Enable(false);
	enemy->dustParticlesGO->SetActive(true);
	enemy->enemyController->Move(enemy->runSpeed, refreshTime, enemy->enemyController->GetPosition(), enemyPath);
}