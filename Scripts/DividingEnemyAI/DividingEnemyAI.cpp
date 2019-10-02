#include "DividingEnemyAI.h"

#include "BasicEnemyAIScript/EnemyState.h"
#include "BasicEnemyAIScript/EnemyStateChase.h"
#include "BasicEnemyAIScript/EnemyStateReturnToStart.h"
#include "BasicEnemyAIScript/EnemyStateAttack.h"
#include "BasicEnemyAIScript/EnemyStateCooldown.h"
#include "BasicEnemyAIScript/EnemyStateFlee.h"
#include "BasicEnemyAIScript/EnemyStateDeath.h"
#include "EnemyStatePatrolDivide.h"
#include "EnemyStateMultiply.h"
#include "EnemyControllerScript/EnemyControllerScript.h"

#include "Application.h"
#include "ModuleTime.h"
#include "ComponentRenderer.h"

DividingEnemyAI_API Script* CreateScript()
{
	DividingEnemyAI* instance = new DividingEnemyAI;
	return instance;
}

void DividingEnemyAI::Start()
{
	enemyStates.reserve(7);

	enemyStates.push_back(patrol = new EnemyStatePatrolDivide(this));
	enemyStates.push_back(chase = new EnemyStateChase(this));
	enemyStates.push_back(returnToStart = new EnemyStateReturnToStart(this));
	enemyStates.push_back(attack = new EnemyStateAttack(this));
	enemyStates.push_back(cooldown = new EnemyStateCooldown(this));
	enemyStates.push_back(death = new EnemyStateDeath(this));
	enemyStates.push_back(flee = new EnemyStateFlee(this));
	enemyStates.push_back(multiply = new EnemyStateMultiply(this));

	currentState = patrol;


	startPosition = enemyController->GetPosition();
}

void DividingEnemyAI::Update()
{
	BasicEnemyAIScript::Update();
}