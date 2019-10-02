#include "SpinToWinEnemyAI.h"

#include "BasicEnemyAIScript/EnemyState.h"
#include "BasicEnemyAIScript/EnemyStateChase.h"
#include "BasicEnemyAIScript/EnemyStateReturnToStart.h"
#include "BasicEnemyAIScript/EnemyStateCooldown.h"
#include "BasicEnemyAIScript/EnemyStateFlee.h"
#include "BasicEnemyAIScript/EnemyStateDeath.h"
#include "BasicEnemyAIScript/EnemyStatePatrol.h"
#include "EnemyStateAttackSpin.h"

#include "EnemyControllerScript/EnemyControllerScript.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleTime.h"
#include "ComponentTransform.h"
#include "GameObject.h"

#define BONE_SPIN "SpinBone"

SpinToWinEnemyAI_API Script* CreateScript()
{
	SpinToWinEnemyAI* instance = new SpinToWinEnemyAI;
	return instance;
}

void SpinToWinEnemyAI::Start()
{
	enemyStates.reserve(7);

	enemyStates.push_back(patrol = new EnemyStatePatrol(this));
	enemyStates.push_back(chase = new EnemyStateChase(this));
	enemyStates.push_back(returnToStart = new EnemyStateReturnToStart(this));
	enemyStates.push_back(attack = new EnemyStateAttackSpin(this));
	enemyStates.push_back(cooldown = new EnemyStateCooldown(this));
	enemyStates.push_back(death = new EnemyStateDeath(this));
	enemyStates.push_back(flee = new EnemyStateFlee(this));

	currentState = patrol;

	startPosition = enemyController->GetPosition();
	spinBones = App->scene->FindGameObjectsByTag(BONE_SPIN, gameobject);
}

void SpinToWinEnemyAI::Update()
{
	BasicEnemyAIScript::Update();
	RotateSpinBone();
}

void SpinToWinEnemyAI::RotateSpinBone()
{
	float rotationAmount = App->time->gameDeltaTime * boneRotationSpeed;
	for (size_t i = 0; i < spinBones.size(); i++)
	{
		spinBones[i]->transform->Rotate(math::float3(0, rotationAmount, 0));
	}
}
