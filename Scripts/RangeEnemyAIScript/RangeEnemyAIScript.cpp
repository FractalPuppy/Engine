#include "RangeEnemyAIScript.h"

#include "Application.h"
#include "ModuleTime.h"
#include "ModuleScene.h"

#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentAnimation.h"
#include "ComponentBoxTrigger.h"

#include "EnemyControllerScript.h"
#include "EnemyState.h"
#include "EnemyStatePatrol.h"
#include "EnemyStateGetInPosition.h"
#include "EnemyStateReturnToStart.h"
#include "EnemyStateAttack.h"
#include "EnemyStateCooldown.h"
#include "EnemyStateDeath.h"

#include "imgui.h"
#include "JSON.h"

RangeEnemyAIScript_API Script* CreateScript()
{
	RangeEnemyAIScript* instance = new RangeEnemyAIScript;
	return instance;
}

void RangeEnemyAIScript::Start()
{
	enemyStates.reserve(6);

	enemyStates.push_back(patrol = new EnemyStatePatrol(this));
	enemyStates.push_back(getInPosition = new EnemyStateGetInPosition(this));
	enemyStates.push_back(returnToStart = new EnemyStateReturnToStart(this));
	enemyStates.push_back(attack = new EnemyStateAttack(this));
	enemyStates.push_back(cooldown = new EnemyStateCooldown(this));
	enemyStates.push_back(death = new EnemyStateDeath(this));
	Appl = App;

	currentState = patrol;

	// Look for Enemy Controller Script of the enemy
	enemyController = gameobject->GetComponent<EnemyControllerScript>();
	if (enemyController == nullptr)
	{
		LOG("The GameObject %s has no Enemy Controller Script component attached \n", gameobject->name);
	}

	boxTrigger = (ComponentBoxTrigger*)gameobject->GetComponentInChildren(ComponentType::BoxTrigger);
	if (boxTrigger == nullptr)
	{
		LOG("No child of the GameObject %s has a boxTrigger component attached \n", gameobject->name);
	}

	startPosition = enemyController->GetPosition();

	LOG("Started basic enemy AI script");
}

void RangeEnemyAIScript::Update()
{
	if (enemyController->GetHealth() <= 0)
	{
		currentState = (EnemyState*)death;
		return;
	}

	EnemyState* previous = currentState;

	if (currentState != death)
	{
		currentState->UpdateTimer();

		currentState->Update();

		// If previous and current are different the functions Exit() and Enter() are called
		CheckStates(previous, currentState);
	}
}

void RangeEnemyAIScript::Expose(ImGuiContext* context)
{
	ImGui::SetCurrentContext(context);
	ImGui::Separator();
	ImGui::Text("Enemy:");

	if (currentState == patrol)				ImGui::TextColored(ImVec4(1, 1, 0, 1), "State: Patrol");
	else if (currentState == getInPosition)	ImGui::TextColored(ImVec4(1, 1, 0, 1), "State: Get In Position");
	else if (currentState == returnToStart)	ImGui::TextColored(ImVec4(1, 1, 0, 1), "State: Return");
	else if (currentState == attack)		ImGui::TextColored(ImVec4(1, 1, 0, 1), "State: Attack");
	else if (currentState == cooldown)		ImGui::TextColored(ImVec4(1, 1, 0, 1), "State: Cooldown");
	else if (currentState == death)			ImGui::TextColored(ImVec4(1, 1, 0, 1), "State: Dead");

	ImGui::Text("Patrol:");
	ImGui::InputFloat("Distance to activate", &activationDistance);
	ImGui::Text("Get In Position:");
	ImGui::InputFloat("Chase Speed", &runSpeed);
	ImGui::InputFloat("Max Attack Distance", &maxAttackDistance);
	ImGui::InputFloat("Min Attack Distance", &minAttackDistance);
	ImGui::Text("Return:");
	ImGui::InputFloat("Disengage Distance", &disengageDistance);
	ImGui::InputFloat("Return Speed", &returnSpeed);
	ImGui::Text("Attack:");
	ImGui::InputFloat("Attack Time", &attackDuration);
	ImGui::InputFloat("Attack Damage", &attackDamage);
	ImGui::Text("Cooldown:");
	ImGui::InputFloat("Cooldown Time", &cooldownTime);
}

void RangeEnemyAIScript::Serialize(JSON_value* json) const
{
	assert(json != nullptr);

	//Wait variables
	json->AddFloat("activationDistance", activationDistance);

	// GetInPosition variables
	json->AddFloat("runSpeed", runSpeed);
	json->AddFloat("maxAttackDistance", maxAttackDistance);
	json->AddFloat("minAttackDistance", minAttackDistance);

	// Return variables
	json->AddFloat("returnSpeed", returnSpeed);
	json->AddFloat("disengageDistance", disengageDistance);

	// Attack variables
	json->AddFloat("attackDuration", attackDuration);
	json->AddFloat("attackDamage", attackDamage);

	// Cooldown variables
	json->AddFloat("cooldownTime", cooldownTime);
}

void RangeEnemyAIScript::DeSerialize(JSON_value* json)
{
	assert(json != nullptr);

	//Wait variables
	activationDistance = json->GetFloat("activationDistance");

	// GetInPosition variables
	runSpeed = json->GetFloat("runSpeed");
	maxAttackDistance = json->GetFloat("maxAttackDistance");
	minAttackDistance = json->GetFloat("minAttackDistance");

	// Return variables
	returnSpeed = json->GetFloat("returnSpeed");
	disengageDistance = json->GetFloat("disengageDistance");

	// Attack variables
	attackDuration = json->GetFloat("attackDuration");
	attackDamage = json->GetFloat("attackDamage");

	// Cooldown variables
	cooldownTime = json->GetFloat("cooldownTime");
}

void RangeEnemyAIScript::CheckStates(EnemyState* previous, EnemyState* current)
{
	if (previous != current)
	{
		previous->ResetTimer();

		previous->Exit();
		current->Enter();

		if (enemyController->anim != nullptr)
		{
			enemyController->anim->SendTriggerToStateMachine(current->trigger.c_str());
		}
	}
}