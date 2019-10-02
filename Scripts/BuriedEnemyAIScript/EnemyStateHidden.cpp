#include "EnemyStateHidden.h"

#include "BuriedEnemyAIScript.h"
#include "EnemyControllerScript.h"

#include "GameObject.h"
#include "ComponentTransform.h"

EnemyStateHidden::EnemyStateHidden(BuriedEnemyAIScript* AIScript)
{
	enemy = AIScript;
	trigger = "Hidden";
}


EnemyStateHidden::~EnemyStateHidden()
{
}


void EnemyStateHidden::HandleIA()
{
	//Check if the player is in Range
	float distanceToPlayer = enemy->enemyController->GetDistanceToPlayer2D();
	if (distanceToPlayer < enemy->activationDistance)
	{
		enemy->currentState = (EnemyState*)enemy->showUp;
	}
}

void EnemyStateHidden::Update()
{
	//nothing
}
