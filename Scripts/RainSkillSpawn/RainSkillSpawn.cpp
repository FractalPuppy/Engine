#include "RainSkillSpawn.h"

#include "Application.h"
#include "ModuleTime.h"

#include "GameObject.h"
#include "ComponentBoxTrigger.h"

#include "EnemyControllerScript.h"

#include "imgui.h"
#include "JSON.h"

RainSkillSpawn_API Script* CreateScript()
{
	RainSkillSpawn* instance = new RainSkillSpawn;
	return instance;
}

void RainSkillSpawn::Start()
{
	if (hits > 0)
		hitDuration = duration / hits;
	else
		hitDuration = duration;

	attackBoxTrigger = gameobject->GetComponent<ComponentBoxTrigger>();
	if (attackBoxTrigger == nullptr)
		LOG("Warning: Missing ComponentBoxTrigger in %s.", gameobject->name.c_str());
}

void RainSkillSpawn::Update()
{
	if (start == false)
	{
		Enable(true);
		start = true;
	}

	timer += App->time->fullGameDeltaTime;

	if (timer < duration)
	{
		Rain();
	}
	else
	{
		Reset();
	}
}

void RainSkillSpawn::Rain()
{
	// Check rain state
	if (attackBoxTrigger != nullptr && attackBoxTrigger->enabled)
	{
		// Hit made?
		if (hitTimer > hitDuration)
		{
			attackBoxTrigger->Enable(false);
		}
		else
		{
			hitTimer += App->time->gameDeltaTime;
		}
	}
	else
	{
		// Go for next hit
		hitTimer = 0.0f;
		attackBoxTrigger->Enable(true);
	}
}

void RainSkillSpawn::Reset()
{
	attackBoxTrigger->Enable(false);
	gameobject->SetActive(false);
	gameobject->deleteFlag = true;
}

void RainSkillSpawn::OnTriggerEnter(GameObject* go)
{
	if (go != nullptr && go->tag == "EnemyHitBox")
	{
		EnemyControllerScript* enemy = go->GetComponent<EnemyControllerScript>();
		if (enemy != nullptr)
		{
			enemy->TakeDamage(10);
		}
		else
		{
			EnemyControllerScript* enemy = go->parent->GetComponent<EnemyControllerScript>();
			if (enemy != nullptr)
			{
				enemy->TakeDamage(10);
			}
			else
			{
				LOG("Error: Enemy script couldn't be found.");
			}
		}
	}
}

void RainSkillSpawn::Expose(ImGuiContext* context)
{
	ImGui::InputInt("Hits number", &hits);
	ImGui::InputFloat("Duration", &duration);
}

void RainSkillSpawn::Serialize(JSON_value* json) const
{
	json->AddInt("hits", hits);
	json->AddFloat("duration", duration);
}

void RainSkillSpawn::DeSerialize(JSON_value* json)
{
	hits = json->GetInt("hits");
	duration = json->GetFloat("duration");
}
