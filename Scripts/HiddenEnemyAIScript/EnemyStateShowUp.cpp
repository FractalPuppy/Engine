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

	if (unborrowParticles)
	{
		unborrowParticles->SetActive(true);
	}

	// Check if the needed Y has been reached
	if (enemy->startPosition.y + enemy->yTranslation <= enemy->enemyController->GetPosition().y)
	{
		if (unborrowParticles)
		{
			unborrowParticles->SetActive(false);
		}
		math::float3 position = enemy->startPosition;
		position.y += enemy->yTranslation;
		enemy->gameobject->transform->SetPosition(position);
		enemy->currentState = (EnemyState*)enemy->chase;
		auxTranslation = 0.0f;
	}
}
