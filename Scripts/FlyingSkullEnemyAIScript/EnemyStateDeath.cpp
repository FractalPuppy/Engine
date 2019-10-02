#include "EnemyStateDeath.h"

#include "EnemyControllerScript.h"
#include "FlyingSkullEnemyAIScript.h"

#include "GameObject.h"
#include "ComponentRenderer.h"

EnemyStateDeath::EnemyStateDeath(FlyingSkullEnemyAIScript* AIScript)
{
	enemy = AIScript;
}


EnemyStateDeath::~EnemyStateDeath()
{
}

void EnemyStateDeath::HandleIA()
{

}

void EnemyStateDeath::Update()
{
	if (timer > waitTime)
	{
		enemy->gameobject->SetActive(false);
	}
}

void EnemyStateDeath::Enter()
{
	for (auto cr : enemy->enemyController->myRenders)
	{
		cr->castShadows = false;
	}
}