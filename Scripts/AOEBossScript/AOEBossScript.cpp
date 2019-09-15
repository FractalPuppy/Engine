#include "Application.h"
#include "ModuleScene.h"
#include "ModuleTime.h"

#include "GameObject.h"
#include "ComponentBoxTrigger.h"

#include "AOEBossScript.h"
#include "PlayerMovement/PlayerMovement.h"

#include "imgui.h"
#include "JSON.h"

AOEBossScript_API Script* CreateScript()
{
	AOEBossScript* instance = new AOEBossScript;
	return instance;
}

void AOEBossScript::Awake()
{
	playerGO = App->scene->FindGameObjectByTag("Player", gameobject->parent);
	if (playerGO == nullptr)
	{
		LOG("The AOE didnt find the playerGO!");
	}
	else
	{
		playerScript = playerGO->GetComponent<PlayerMovement>();
		if (playerScript == nullptr)
		{
			LOG("The AOE didnt find the playerScript!");
		}
	}
}

void AOEBossScript::Start()
{
	prepParticlesGO = App->scene->FindGameObjectByName("Prep Particles", gameobject);
	if (prepParticlesGO == nullptr)
	{
		LOG("PrepParticles not found");
	}
	beamParticlesGO = App->scene->FindGameObjectByName("Beam Particles", gameobject);
	if (beamParticlesGO == nullptr)
	{
		LOG("beamParticlesGO not found");
	}
	boxTriggerGO = App->scene->FindGameObjectByName("Hitbox", gameobject);
	if (boxTriggerGO == nullptr)
	{
		LOG("boxTriggerGO not found");
	}
	else
	{
		boxTrigger = boxTriggerGO->GetComponent<ComponentBoxTrigger>();
		if (boxTrigger == nullptr)
		{
			LOG("boxTrigger not found");
		}
	}

	boxTrigger->Enable(false);
	boxTriggerGO->SetActive(false);
	prepParticlesGO->SetActive(true);
	beamParticlesGO->SetActive(false);
}

void AOEBossScript::Update()
{
	timer += App->time->gameDeltaTime;

	if (timer > timerFade && timer < duration)
	{
		beamParticlesGO->SetActive(true);
		prepParticlesGO->SetActive(false);
		boxTriggerGO->SetActive(true);
		boxTrigger->Enable(true);
	}
	else if (timer > duration)
	{
		boxTriggerGO->SetActive(false);
		beamParticlesGO->SetActive(false);
		boxTrigger->Enable(false);
		gameobject->deleteFlag = true;
	}
}

void AOEBossScript::OnTriggerEnter(GameObject * go)
{
	if (go == playerGO && !hasDamaged)
	{
		playerScript->Damage(damageToPlayer);
		hasDamaged = true;
	}
}

void AOEBossScript::Expose(ImGuiContext * context)
{
	ImGui::DragFloat("Time until particles change", &timerFade,0.1f,0.0f,20.0f);
	ImGui::DragFloat("duration", &duration, 0.1f, 1.0f, 20.0f);
	ImGui::InputFloat("Damage", &damageToPlayer);
}

void AOEBossScript::Serialize(JSON_value * json) const
{
	json->AddFloat("timerFade", timerFade);
	json->AddFloat("duration", duration);
	json->AddFloat("damageToPlayer", damageToPlayer);
}

void AOEBossScript::DeSerialize(JSON_value * json)
{
	timerFade = json->GetFloat("timerFade", 0.0f);
	duration = json->GetFloat("duration", 0.0f);
	damageToPlayer = json->GetFloat("damageToPlayer", 0.0f);
}
