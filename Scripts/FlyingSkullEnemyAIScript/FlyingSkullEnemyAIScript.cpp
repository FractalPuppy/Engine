#include "FlyingSkullEnemyAIScript.h"

#include "EnemyControllerScript.h"
#include "ProjectileScript.h"

#include "EnemyStateAttack.h"
#include "EnemyStateChase.h"
#include "EnemyStateCooldown.h"
#include "EnemyStateDeath.h"
#include "EnemyStatePatrol.h"
#include "EnemyStateFlee.h"
#include "EnemyStateReturn.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleTime.h"

#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentRenderer.h"

#include "imgui.h"
#include <assert.h>
#include "JSON.h"

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

	modifiedSpeed = idleMovementSpeed;

	boneCenter = App->scene->FindGameObjectByName("BoneCenter", gameobject);
	assert(boneCenter != nullptr);
	if (boneCenter == nullptr)
	{
		LOG("No BoneCenter GO found");
	}

	firstSpin = App->scene->FindGameObjectByName("FirstSpinning", gameobject);
	assert(firstSpin != nullptr);
	if (firstSpin == nullptr)
	{
		LOG("No firstSpin GO found");
	}
	else
	{
		firstBone = App->scene->FindGameObjectByName("Bone", firstSpin);
		assert(firstBone != nullptr);
		if (firstBone == nullptr)
		{
			LOG("No first bone GO found");
		}

		firstRotation = App->scene->FindGameObjectByName("Rotation", firstSpin);
		assert(firstRotation != nullptr);
		if (firstRotation == nullptr)
		{
			LOG("No first rotaiton GO found");
		}
	}

	secondSpin = App->scene->FindGameObjectByName("SecondSpinning", gameobject);
	assert(secondSpin != nullptr);
	if (secondSpin == nullptr)
	{
		LOG("No secondSpin GO found");
	}
	else
	{
		secondBone = App->scene->FindGameObjectByName("Bone", secondSpin);
		assert(secondBone != nullptr);
		if (secondBone == nullptr)
		{
			LOG("No second bone GO found");
		}

		secondRotation = App->scene->FindGameObjectByName("Rotation", secondSpin);
		assert(secondRotation != nullptr);
		if (secondRotation == nullptr)
		{
			LOG("No second rotaiton GO found");
		}
	}

	projectileGO = App->scene->FindGameObjectByName("Projectile", gameobject->parent);
	assert(projectileGO != nullptr);
	if (projectileGO != nullptr)
	{
		projectileScript = projectileGO->GetComponent<ProjectileScript>();
		if (projectileScript == nullptr)
		{
			LOG("no projectile script found");
		}
	}
	else
	{
		LOG("no projectile GO found");
	}
	

	particles = App->scene->FindGameObjectByName("Particles", gameobject);
	//assert(particles != nullptr);
	if (particles == nullptr)
	{
		LOG("No particles GO found");
	}

	LOG("Started Flying Skull script");
}

void FlyingSkullEnemyAIScript::Update()
{
	if (enemyController->GetHealth() <= 0 && currentState != death)
	{
		currentState = (EnemyState*)death;
	}

	if (currentState != death)
	{
		Movement();
		OrbitMovement();
	}

	EnemyState* previous = currentState;

	currentState->UpdateTimer();
	currentState->HandleIA();

	CheckStates(previous);

	currentState->Update();
}

void FlyingSkullEnemyAIScript::Expose(ImGuiContext * context)
{

	if (currentState == patrol)				ImGui::TextColored(ImVec4(1, 1, 0, 1), "State: patrol");
	else if (currentState == chase)			ImGui::TextColored(ImVec4(1, 1, 0, 1), "State: Chase");
	else if (currentState == returnToStart)	ImGui::TextColored(ImVec4(1, 1, 0, 1), "State: Return");
	else if (currentState == attack)		ImGui::TextColored(ImVec4(1, 1, 0, 1), "State: Attack");
	else if (currentState == cooldown)		ImGui::TextColored(ImVec4(1, 1, 0, 1), "State: Cooldown");
	else if (currentState == death)			ImGui::TextColored(ImVec4(1, 1, 0, 1), "State: Dead");
	else if (currentState == flee)			ImGui::TextColored(ImVec4(1, 1, 0, 1), "State: flee");


	ImGui::InputFloat("attack cooldown duration: ", &cooldownDuration);
	ImGui::InputFloat("Flee duration: ", &fleeDuration);
	ImGui::InputFloat("Attack duration: ", &attackDuration);
	ImGui::InputFloat("activation distance: ", &activationDistance);
	ImGui::InputFloat("disengage distance: ", &disengageDistance);
	ImGui::InputFloat("max attack range: ", &maxAttackRange);
	ImGui::InputFloat("min attack range: ", &minAttackRange);
	ImGui::InputFloat("Projectile Delay: ", &projectileDelay);
	ImGui::InputFloat("run speed: ", &runSpeed);
	ImGui::Separator();
	ImGui::Text("Animation values");
	ImGui::InputFloat("Movement constant: ", &movementConstant);
	ImGui::InputFloat("Max idle speed: ", &maxIdleSpeed);
	ImGui::InputFloat("Min idle speed: ", &minIdleSpeed);
	if (ImGui::InputFloat("Idle movement speed: ", &idleMovementSpeed))
	{
		modifiedSpeed = idleMovementSpeed;
		acceleration = 0.0f;
	}
}

void FlyingSkullEnemyAIScript::Serialize(JSON_value * json) const
{
	json->AddFloat("cooldownDuration", cooldownDuration);
	json->AddFloat("fleeDuration", fleeDuration);
	json->AddFloat("attackDuration", attackDuration);
	json->AddFloat("activationDistance", activationDistance);
	json->AddFloat("disengageDistance", disengageDistance);
	json->AddFloat("maxAttackRange", maxAttackRange);
	json->AddFloat("minAttackRange", minAttackRange);
	json->AddFloat("projectileDelay", projectileDelay);
	json->AddFloat("runSpeed", runSpeed);
	json->AddFloat("idleMovementSpeed", idleMovementSpeed);
	json->AddFloat("maxIdleSpeed", maxIdleSpeed);
	json->AddFloat("minIdleSpeed", minIdleSpeed);
	json->AddFloat("acceleration", acceleration);
	json->AddFloat("movementConstant", movementConstant);
}

void FlyingSkullEnemyAIScript::DeSerialize(JSON_value * json)
{
	cooldownDuration = json->GetFloat("cooldownDuration", 0);
	fleeDuration = json->GetFloat("fleeDuration", 0);
	attackDuration = json->GetFloat("attackDuration", 0);
	activationDistance = json->GetFloat("activationDistance",0);
	disengageDistance = json->GetFloat("disengageDistance",0);
	maxAttackRange = json->GetFloat("maxAttackRange",0);
	minAttackRange = json->GetFloat("minAttackRange",0);
	projectileDelay = json->GetFloat("projectileDelay", 0);
	idleMovementSpeed = json->GetFloat("idleMovementSpeed", 0);
	minIdleSpeed = json->GetFloat("minIdleSpeed", 0);
	maxIdleSpeed = json->GetFloat("maxIdleSpeed", 0);
	acceleration = json->GetFloat("acceleration", 0);
	movementConstant = json->GetFloat("movementConstant", 0);
	runSpeed = json->GetFloat("runSpeed",0);
}

void FlyingSkullEnemyAIScript::Movement()
{
	ComponentTransform* transform = boneCenter->transform;

	acceleration = sin(angle);

	modifiedSpeed += acceleration * App->time->gameDeltaTime;

	if (modifiedSpeed > maxIdleSpeed)
	{
		modifiedSpeed = maxIdleSpeed;
	}
	else if (modifiedSpeed < minIdleSpeed)
	{
		modifiedSpeed = minIdleSpeed;
	}

	angle += modifiedSpeed * App->time->gameDeltaTime;
	float cosx = cos(angle);
	float radius = movementConstant * cos(3 * angle); // the rose will have 3 petals

	float z = radius * cos(angle);
	float y = radius * sin(angle);

	transform->SetPosition({ 0, y + offsetHeight,z });
}

void FlyingSkullEnemyAIScript::OrbitMovement()
{
	//First Orbit
	math::Quat rotationDeltaA = math::Quat::identity;

	rotationDeltaA.z += firstRotationSpeed;
	rotationDeltaA.Normalize();

	firstRotation->transform->SetRotation(firstRotation->transform->GetRotation()*rotationDeltaA);

	//Second Orbit
	math::Quat rotationDeltaB = math::Quat::identity;

	rotationDeltaB.z += secondRotationSpeed;
	rotationDeltaB.Normalize();

	secondRotation->transform->SetRotation(secondRotation->transform->GetRotation()*rotationDeltaB);

	//First bone
	rotationDeltaB.x += secondRotationSpeed;
	rotationDeltaB.Normalize();
	firstBone->transform->SetRotation(firstBone->transform->GetRotation()*rotationDeltaB);

	//Second bone
	rotationDeltaA.x += firstRotationSpeed;
	rotationDeltaA.Normalize();
	secondBone->transform->SetRotation(secondBone->transform->GetRotation()*rotationDeltaA);

}

void FlyingSkullEnemyAIScript::CheckStates(EnemyState* previous)
{
	if (previous != currentState)
	{
		previous->ResetTimer();
		previous->Exit();
		currentState->Enter();
	}
}
