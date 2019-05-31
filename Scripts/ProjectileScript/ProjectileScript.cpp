#include "ProjectileScript.h"

#include "Application.h"
#include "ModuleTime.h"
#include "ModuleScene.h"

#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentBoxTrigger.h"
#include "ComponentRenderer.h"

#include "PlayerMovement/PlayerMovement.h"

#include "imgui.h"
#include "JSON.h"

ProjectileScript_API Script* CreateScript()
{
	ProjectileScript* instance = new ProjectileScript;
	return instance;
}

void ProjectileScript::Start()
{
	transform = gameobject->transform;
	if (transform == nullptr)
	{
		LOG("The transform of the GameObject %s couldn't be found. \n", gameobject->name);
	}

	boxTrigger = gameobject->GetComponent<ComponentBoxTrigger>();
	if (boxTrigger == nullptr)
	{
		LOG("The GameObject %s doesn't have a boxTrigger component attached. \n", gameobject->name);
	}

	// Player
	player = App->scene->FindGameObjectByName(playerName.c_str());
	if (player == nullptr)
	{

	}
	else
	{
		playerScript = player->GetComponent<PlayerMovement>();
		if (playerScript == nullptr)
		{
			LOG("The GameObject %s doesn't have a Player Movement component attached. \n", gameobject->name);
		}
	}
}

void ProjectileScript::Update()
{
	if (shooted)
	{
		float deltaTime = App->time->gameDeltaTime;
		activeTime += deltaTime;
		// Move forward
		math::float3 movement = transform->front.Normalized() * -speed * deltaTime;
		transform->SetPosition(gameobject->transform->GetPosition() + movement);

		if ((activeTime - auxTimer) > lifeTime)
		{
			gameobject->SetActive(false);
			ComponentRenderer* mesh = gameobject->GetComponent<ComponentRenderer>();
			if (mesh != nullptr && !mesh->enabled)
				mesh->Enable(true);
			shooted = false;
		}
	}
	else
	{
		auxTimer = App->time->gameDeltaTime;
		activeTime = 0.0f;
		shooted = true;
	}
}

void ProjectileScript::OnTriggerEnter(GameObject* go)
{
	if (player != nullptr && go == player)
	{
		if(playerScript != nullptr)
			playerScript->Damage(10);

		//gameobject->SetActive(false);
		ComponentRenderer* mesh = gameobject->GetComponent<ComponentRenderer>();
		if(mesh != nullptr)
			mesh->Enable(false);
		shooted = false;
	}
}

void ProjectileScript::Expose(ImGuiContext* context)
{
	ImGui::InputFloat("Projectile speed", &speed);
	ImGui::InputFloat("Life time", &lifeTime);

	char* targetName = new char[64];
	strcpy_s(targetName, strlen(playerName.c_str()) + 1, playerName.c_str());
	ImGui::InputText("Player Name", targetName, 64);
	playerName = targetName;
	delete[] targetName;
}

void ProjectileScript::Serialize(JSON_value* json) const
{
	json->AddFloat("speed", speed);
	json->AddFloat("lifeTime", lifeTime);
	json->AddString("playerName", playerName.c_str());
}

void ProjectileScript::DeSerialize(JSON_value* json)
{
	speed = json->GetFloat("speed");
	lifeTime = json->GetFloat("lifeTime");
	playerName = json->GetString("playerName");
}

