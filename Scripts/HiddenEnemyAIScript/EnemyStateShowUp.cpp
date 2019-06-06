#include "EnemyStateShowUp.h"

#include "GameObject.h"
#include "ComponentTransform.h"

#include "HiddenEnemyAIScript.h"
#include "EnemyControllerScript.h"

EnemyStateShowUp::EnemyStateShowUp(HiddenEnemyAIScript* AIScript)
{
	enemy = AIScript;
	trigger = "ShowUp";
}

EnemyStateShowUp::~EnemyStateShowUp()
{
}

void EnemyStateShowUp::Update()
{
	// Translate on the Y axis
	enemy->enemyController->Move(enemy->showUpSpeed, enemy->gameobject->transform->up);

	// Check if the needed Y has been reached
	if (enemy->startPosition.y <= enemy->enemyController->GetPosition().y)
	{
		enemy->gameobject->transform->SetPosition(enemy->startPosition);
		enemy->currentState = (EnemyState*)enemy->chase;
		auxTranslation = 0.0f;
	}
}
