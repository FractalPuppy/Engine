#include "EnemyStateHide.h"

#include "EnemyControllerScript.h"
#include "BuriedEnemyAIScript.h"

#include "GameObject.h"
#include "ComponentRenderer.h"
#include "ComponentBoxTrigger.h"
#include "ComponentTransform.h"

EnemyStateHide::EnemyStateHide(BuriedEnemyAIScript* AIScript)
{
	enemy = AIScript;
	trigger = "Hide";
}


EnemyStateHide::~EnemyStateHide()
{
}

void EnemyStateHide::HandleIA()
{
	if (timer > duration)
	{
		enemy->currentState = (EnemyState*)enemy->hidden;
	}
}

void EnemyStateHide::Update()
{

}

void EnemyStateHide::Exit()
{
	// Set candle on the ground
	float candleZPos = enemy->candleGO->transform->position.z;
	if(candleZPos > candleZPos - enemy->candleOffset)
		enemy->candleGO->transform->position.z -= enemy->candleOffset;

	enemy->enemyController->GetMainRenderer()->Enable(false);
}

void EnemyStateHide::Enter()
{
	enemy->enemyController->hpBoxTrigger->Enable(false);
}
