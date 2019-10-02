#include "EnemyStateShowUp.h"

#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentRenderer.h"

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

void EnemyStateShowUp::Enter()
{
	enemy->enemyController->GetMainRenderer()->Enable(true);
}

void EnemyStateShowUp::HandleIA()
{
	if (timer > enemy->currentState->duration)
	{
		enemy->currentState = (EnemyState*)enemy->chase;
	}
}

void EnemyStateShowUp::Update()
{

}
