#include "Application.h"
#include "ModuleScene.h"
#include "ModuleTime.h"

#include "GameObject.h"
#include "ComponentRenderer.h"
#include "ComponentBoxTrigger.h"
#include "ComponentTransform.h"

#include "BossProjectileScript.h"
#include "PlayerMovement/PlayerMovement.h"

#include "imgui.h"
#include "JSON.h"

BossProjectileScript_API Script* CreateScript()
{
	BossProjectileScript* instance = new BossProjectileScript;
	return instance;
}

void BossProjectileScript::Awake()
{
	playerGO = App->scene->FindGameObjectByTag("Player", gameobject->parent);
	if (playerGO == nullptr)
	{
		LOG("Player in bossProjectile not found");
	}
	else
	{
		playerScript = playerGO->GetComponent<PlayerMovement>();
		if (playerScript == nullptr)
		{
			LOG("Playerscript in bossProjectile not found");
		}
		render = (ComponentRenderer*)playerGO->GetComponentInChildren(ComponentType::Renderer);
		if (render == nullptr)
		{
			LOG("Render in bossProjectile not found")
		}
		trigger = (ComponentBoxTrigger*)playerGO->GetComponentInChildren(ComponentType::BoxTrigger);
		if (trigger == nullptr)
		{
			LOG("BoxTrigger in bossProjectile not found");
		}
	}

}

void BossProjectileScript::Start()
{
	render->dissolve = true;
	render->dissolveAmount = 1.0f;
	trigger->Enable(false);
}

void BossProjectileScript::Update()
{
	math::float3 playerPosition = playerGO->transform->GetPosition();

	
	//Having a switch is faster but this also works :p
	if (!fadeInComplete)
	{
		render->dissolveAmount -= App->time->gameDeltaTime;
		if (render->dissolveAmount >= 0.0f)
		{
			fadeInComplete = true;
		}
	}
	else if (fadeInComplete && !waitingComplete)
	{
		gameobject->transform->LookAt(playerPosition + math::float3(0.0f, 200.0f, 0.0f));
		waitTimer += App->time->gameDeltaTime;
		if (waitTimer > waitingTime)
		{
			shootingDirection = (playerPosition + math::float3(0.0f, 200.0f, 0.0f)) - gameobject->transform->GetPosition();
			shootingDirection.Normalize();

			waitingComplete = true;
		}
	}
	else
	{
		//Go to the player
		math::float3 currentPosition = gameobject->transform->GetPosition();
		gameobject->transform->SetPosition(currentPosition + shootingDirection * projectileSpeed);
	}
}

void BossProjectileScript::OnTriggerEnter(GameObject * go)
{
	if (go == playerGO && !hasDamaged)
	{
		playerScript->Damage(damageAmount);
	}
}

void BossProjectileScript::Expose(ImGuiContext * context)
{
	ImGui::DragFloat("Waiting time", &waitingTime, 0.1f, 0.1f, 10.0f);
	ImGui::DragFloat("Projectile speed", &projectileSpeed, 10.0f, 10.0f, 5000.0f);
	ImGui::InputFloat("Damage", &damageAmount);
}

void BossProjectileScript::Serialize(JSON_value * json) const
{
	json->AddFloat("waitingTime", waitingTime);
	json->AddFloat("projectileSpeed", projectileSpeed);
	json->AddFloat("damageAmount", damageAmount);
}

void BossProjectileScript::DeSerialize(JSON_value * json)
{
	waitingTime = json->GetFloat("waitingTime",0.0f);
	projectileSpeed = json->GetFloat("projectileSpeed", 0.0f);
	damageAmount = json->GetFloat("damageAmount", 0.0f);
}
