#include "Application.h"
#include "ModuleTime.h"

#include "GameObject.h"

#include "BossBehaviourScript.h"
#include "EnemyControllerScript/EnemyControllerScript.h"

#include "BossState.h"
#include "BossStateIdle.h"
#include "BossStateNotActive.h"
#include "BossStateActivated.h"
#include "BossStateDeath.h"
#include "BossStateCast.h"
#include "BossStatePreCast.h"
#include "BossStateInterPhase.h"

#include "imgui.h"
#include "JSON.h"

BossBehaviourScript_API Script* CreateScript()
{
	BossBehaviourScript* instance = new BossBehaviourScript;
	return instance;
}

void BossBehaviourScript::Awake()
{
	// Look for Enemy Controller Script of the enemy
	enemyController = gameobject->GetComponent<EnemyControllerScript>();
	if (enemyController == nullptr)
	{
		LOG("The GameObject %s has no Enemy Controller Script component attached \n", gameobject->name.c_str());
	}

}

void BossBehaviourScript::Start()
{
	startingPoint = enemyController->GetPosition();
	bossPhase = BossPhase::None;

	bossStates.reserve(7);
	bossStates.push_back(notActive = new BossStateNotActive(this));
	bossStates.push_back(activated = new BossStateActivated(this));
	bossStates.push_back(idle = new BossStateIdle(this));
	bossStates.push_back(cast = new BossStateCast(this));
	bossStates.push_back(precast = new BossStatePreCast(this));
	bossStates.push_back(interPhase = new BossStateInterPhase(this));
	bossStates.push_back(death = new BossStateDeath(this));

	currentState = notActive;
}

void BossBehaviourScript::Update()
{
	distanceToPlayer = ReturnDistanceToPlayer();
	BossState* previous = currentState;

	FloatInSpace();

	
	currentState->HandleIA();
	CheckHealth();

	CheckStates(previous);
	currentState->UpdateTimer();
	currentState->Update();
	
}

void BossBehaviourScript::Expose(ImGuiContext * context)
{
	//Boss state check
	if (currentState == notActive)				ImGui::TextColored(ImVec4(1, 1, 0, 1), "State: Not active");
	else if (currentState == activated)			ImGui::TextColored(ImVec4(1, 1, 0, 1), "State: Activated");
	else if (currentState == death)				ImGui::TextColored(ImVec4(1, 1, 0, 1), "State: Dead");
	else if (currentState == interPhase)		ImGui::TextColored(ImVec4(1, 1, 0, 1), "State: interPhase");
	else if (currentState == precast)			ImGui::TextColored(ImVec4(1, 1, 0, 1), "State: Precast");
	else if (currentState == cast)				ImGui::TextColored(ImVec4(1, 1, 0, 1), "State: Cast");
	else if (currentState == idle)				ImGui::TextColored(ImVec4(1, 1, 0, 1), "State: idle");

	if (bossPhase == BossPhase::First)			ImGui::TextColored(ImVec4(0, 1, 0, 1), "First phase");
	else if(bossPhase == BossPhase::Second)		ImGui::TextColored(ImVec4(0, 1, 0, 1), "Second phase");
	else if (bossPhase == BossPhase::Third)		ImGui::TextColored(ImVec4(0, 1, 0, 1), "Third phase");


	ImGui::Text("Floating parameters:");
	ImGui::DragFloat("floating constant:", &floatConstant, 0.1f,0.0f,10.0f);
	ImGui::DragFloat("angle constant:", &angleConstant, 0.1f, 0.0f, 10.0f);

	ImGui::Separator();
	ImGui::Text("Positions first cutscene");
	ImGui::InputFloat3("HighPoint position", (float*)&highPointFirstCS);
	ImGui::InputFloat3("Start Fight position", (float*)&pointStartFight);

	ImGui::Separator();
	ImGui::Text("Fight parameters");
	ImGui::DragFloat("Activation Distance", &activationDistance, 10.0f,500.0f,10000.0f);
	ImGui::DragFloat("Activation time", &activationTime, 0.1f, 0.5f, 20.0f);
}

void BossBehaviourScript::Serialize(JSON_value * json) const
{
	json->AddFloat("floatConstant", floatConstant);
	json->AddFloat("angleConstant", angleConstant);
	json->AddFloat("activationDistance", activationDistance);
	json->AddFloat("activationTime", activationTime);

	json->AddFloat3("highPointFirstCS", highPointFirstCS);
	json->AddFloat3("pointStartFight", pointStartFight);
}

void BossBehaviourScript::DeSerialize(JSON_value * json)
{
	floatConstant = json->GetFloat("floatConstant", 0.0f);
	angleConstant = json->GetFloat("angleConstant", 0.0f);
	activationDistance = json->GetFloat("activationDistance", 0.0f);
	activationTime = json->GetFloat("activationTime", 0.0f);
	highPointFirstCS = json->GetFloat3("highPointFirstCS");
	pointStartFight = json->GetFloat3("pointStartFight");
}

void BossBehaviourScript::CheckStates(BossState * previous)
{
	if (previous != currentState)
	{
		previous->ResetTimer();

		previous->Exit();
		currentState->Enter();
	}
}

void BossBehaviourScript::CheckHealth()
{
	float actualHealth = enemyController->GetHealth();
	float maxHealth = enemyController->GetMaxHealth();

	float healthPerc = actualHealth / maxHealth;
	switch (bossPhase)
	{
		case BossPhase::None:
			break;
		case BossPhase::First:
			if (healthPerc < firstHealthThreshold)
			{
				bossPhase = BossPhase::Second;
			}
			break;
		case BossPhase::Second:
			if (healthPerc < secondHealthThreshold)
			{
				bossPhase = BossPhase::Third;
				currentState = interPhase;
				isFloating = false;
			}
			break;
		case BossPhase::Third:
			break;
	}


}

void BossBehaviourScript::FloatInSpace()
{
	if (isFloating)
	{
		yOffset = floatConstant * sin(angle);
		angle += angleConstant * App->time->gameDeltaTime;

		math::float3 enemyPosition = enemyController->GetPosition();
		enemyController->SetPosition({ enemyPosition.x,enemyPosition.y + yOffset,enemyPosition.z });
	}
}

float BossBehaviourScript::ReturnDistanceToPlayer()
{
	return enemyController->GetDistanceToPlayer2D();
}
