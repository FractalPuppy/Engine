#include "CombatAudioEvents.h"
#include "Application.h"

#include "ModuleScene.h"
#include "GameObject.h"

#include "ComponentAudioSource.h"

CombatAudioEvents_API Script* CreateScript()
{
	CombatAudioEvents* instance = new CombatAudioEvents;
	return instance;
}

void CombatAudioEvents::Start()
{
	GameObject* GO = nullptr;

	GO = App->scene->FindGameObjectByName("stepSound");
	if (GO != nullptr)
	{
		stepSound = GO->GetComponent<ComponentAudioSource>();
		assert(stepSound != nullptr);
	}
	else
	{
		LOG("Warning: stepSound game object not found");
	}

	GO = App->scene->FindGameObjectByName("bomb_take_off");
	if (GO != nullptr)
	{
		bomb_take_off = GO->GetComponent<ComponentAudioSource>();
		assert(bomb_take_off != nullptr);
	}
	else
	{
		LOG("Warning: bomb_take_off game object not found");
	}

	GO = App->scene->FindGameObjectByName("bomb_impact");
	if (GO != nullptr)
	{
		bomb_impact = GO->GetComponent<ComponentAudioSource>();
		assert(bomb_impact != nullptr);
	}
	else
	{
		LOG("Warning: bomb_impact game object not found");
	}

	GO = App->scene->FindGameObjectByName("attack1");
	if (GO != nullptr)
	{
		attack1 = GO->GetComponent<ComponentAudioSource>();
		assert(attack1 != nullptr);
	}
	else
	{
		LOG("Warning: attack1 game object not found");
	}
	
	GO = App->scene->FindGameObjectByName("attack2");
	if (GO != nullptr)
	{
		attack2 = GO->GetComponent<ComponentAudioSource>();
		assert(attack2 != nullptr);
	}
	else
	{
		LOG("Warning: attack2 game object not found");
	}

	GO = App->scene->FindGameObjectByName("spin_attack");
	if (GO != nullptr)
	{
		spin_attack = GO->GetComponent<ComponentAudioSource>();
		assert(spin_attack != nullptr);
	}
	else
	{
		LOG("Warning: spin_attack game object not found");
	}

	GO = App->scene->FindGameObjectByName("drill_attack");
	if (GO != nullptr)
	{
		drill_attack = GO->GetComponent<ComponentAudioSource>();
		assert(drill_attack != nullptr);
	}
	else
	{
		LOG("Warning: drill_attack game object not found");
	}

	GO = App->scene->FindGameObjectByName("enemy_got_hit");
	if (GO != nullptr)
	{
		enemy_got_hit = GO->GetComponent<ComponentAudioSource>();
		assert(enemy_got_hit != nullptr);
	}
	else
	{
		LOG("Warning: enemy_got_hit game object not found");
	}
}

void CombatAudioEvents::OnAnimationEvent(std::string name)
{
	if (name == "step")
	{
		float offset = randomOffset(0.2) - 0.1;
		stepSound->SetPitch(0.7 + offset);
		stepSound->Play();

	}	
	else if (name == "bomb_take_off")
	{
		bomb_take_off->Play();
	}
	else if (name == "bomb_impact")
	{
		bomb_impact->Play();
	}
	else if (name == "attack1")
	{
		float offset = randomOffset(0.2) - 0.1;
		attack1->SetPitch(0.9 + offset);
		attack1->Play();
	}
	else if (name == "attack2")
	{
		float offset = randomOffset(0.2) - 0.1;
		attack2->SetPitch(0.9 + offset);
		attack2->Play();
	}
	else if (name == "spin_attack")
	{
		spin_attack->Play();
	}
	else if (name == "drill_attack")
	{
		drill_attack->Play();
	}
}

void CombatAudioEvents::enemyGotHit(int i)
{
	float offset = randomOffset(0.3) - 0.15;
	if (enemy_got_hit != nullptr)
	{
		enemy_got_hit->SetPitch(0.9 + offset);
		enemy_got_hit->Play();
	}
}

float CombatAudioEvents::randomOffset(float max)
{
	float random = rand() % (int)(max * 100);
	return (float)random / 100.f;
	
}


