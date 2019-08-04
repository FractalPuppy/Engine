#include "FlyingSkullEnemyAIScript.h"

#include "EnemyControllerScript.h"

#include "EnemyStateAttack.h"
#include "EnemyStateChase.h"
#include "EnemyStateCooldown.h"
#include "EnemyStateDeath.h"
#include "EnemyStatePatrol.h"
#include "EnemyStateFlee.h"
#include "EnemyStateReturn.h"

#include "GameObject.h"

FlyingSkullEnemyAIScript_API Script* CreateScript()
{
	FlyingSkullEnemyAIScript* instance = new FlyingSkullEnemyAIScript;
	return instance;
}

void FlyingSkullEnemyAIScript::Awake()
{
	// Look for Enemy Controller Script of the enemy
	enemyController = gameobject->GetComponent<EnemyControllerScript>();
	if (enemyController == nullptr)
	{
		LOG("The GameObject %s has no Enemy Controller Script component attached \n", gameobject->name);
	}
}

void FlyingSkullEnemyAIScript::Start()
{
	//Create states
	enemyStates.reserve(7);
	enemyStates.push_back(attack		= new EnemyStateAttack(this));
	enemyStates.push_back(cooldown		= new EnemyStateCooldown(this));
	enemyStates.push_back(returnToStart = new EnemyStateReturn(this));
	enemyStates.push_back(chase			= new EnemyStateChase(this));
	enemyStates.push_back(death			= new EnemyStateDeath(this));
	enemyStates.push_back(flee			= new EnemyStateFlee(this));
	enemyStates.push_back(patrol		= new EnemyStatePatrol(this));

	currentState = patrol;

	startPosition = enemyController->GetPosition();
}

void FlyingSkullEnemyAIScript::Update()
{
}

void FlyingSkullEnemyAIScript::Expose(ImGuiContext * context)
{
}

void FlyingSkullEnemyAIScript::Serialize(JSON_value * json) const
{
}

void FlyingSkullEnemyAIScript::DeSerialize(JSON_value * json)
{
}
