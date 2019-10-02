#include "EnemyStateHide.h"

#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentRenderer.h"

#include "HiddenEnemyAIScript.h"
#include "EnemyControllerScript.h"

EnemyStateHide::EnemyStateHide(HiddenEnemyAIScript* AIScript)
{
	enemy = AIScript;
	trigger = "Hide";
}


EnemyStateHide::~EnemyStateHide()
{
}

void EnemyStateHide::Exit()
{
	enemy->enemyController->GetMainRenderer()->Enable(false);
}

void EnemyStateHide::HandleIA()
{
	if (timer > enemy->currentState->duration)
	{
		enemy->currentState = (EnemyState*)enemy->wait;
	}
}

void EnemyStateHide::Update()
{
}
