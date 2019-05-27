#include "EnemyControllerScript.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleTime.h"

#include "GameObject.h"
#include "ComponentRenderer.h"
#include "ComponentTransform.h"

#include "imgui.h"
#include "JSON.h"

EnemyControllerScript_API Script* CreateScript()
{
	EnemyControllerScript* instance = new EnemyControllerScript;
	return instance;
}

void EnemyControllerScript::Start()
{
	// Look for enemy BBox
	GameObject* enemyBBox = App->scene->FindGameObjectByName(gameobject, myBboxName.c_str());
	if (enemyBBox == nullptr)
	{
		LOG("The GO %s couldn't be found \n", myBboxName);
	}
	else
	{
		myBbox = &enemyBBox->bbox;
		if (myBbox == nullptr)
		{
			LOG("The GameObject %s has no bbox attached \n", enemyBBox->name);
		}
	}

	// Look for player and his BBox
	player = App->scene->FindGameObjectByName(App->scene->root, playerName.c_str());
	if (player == nullptr)
	{
		LOG("The GO %s couldn't be found \n", playerName);
	}
	else
	{
		GameObject* playerMesh = App->scene->FindGameObjectByName(player, playerBboxName.c_str());
		if (playerMesh != nullptr)
		{
			playerBbox = &playerMesh->bbox;
			if (playerBbox == nullptr)
			{
				LOG("The GameObject %s has no bbox attached \n", player->name);
			}
		}
	}

	// Look for Component Animation of the enemy
	anim = (ComponentAnimation*)gameobject->GetComponentInChildren(ComponentType::Animation);
	if (anim == nullptr)
	{
		LOG("No child of the GameObject %s has an Animation component attached \n", gameobject->name);
	}
}

void EnemyControllerScript::Expose(ImGuiContext * context)
{
	char* bboxName = new char[64];
	strcpy_s(bboxName, strlen(myBboxName.c_str()) + 1, myBboxName.c_str());
	ImGui::InputText("My BBox Name", bboxName, 64);
	myBboxName = bboxName;
	delete[] bboxName;

	ImGui::InputInt("Health", &health);

	ImGui::Separator();
	ImGui::Text("Player:");
	char* goName = new char[64];
	strcpy_s(goName, strlen(playerName.c_str()) + 1, playerName.c_str());
	ImGui::InputText("playerName", goName, 64);
	playerName = goName;
	delete[] goName;

	char* targetBboxName = new char[64];
	strcpy_s(targetBboxName, strlen(playerBboxName.c_str()) + 1, playerBboxName.c_str());
	ImGui::InputText("Player BBox Name", targetBboxName, 64);
	playerBboxName = targetBboxName;
	delete[] targetBboxName;
}

void EnemyControllerScript::Serialize(JSON_value* json) const
{
	assert(json != nullptr);
	json->AddString("playerName", playerName.c_str());
	json->AddString("playerBboxName", playerBboxName.c_str());
	json->AddString("myBboxName", myBboxName.c_str());
	json->AddInt("health", health);
}

void EnemyControllerScript::DeSerialize(JSON_value* json)
{
	assert(json != nullptr);
	playerName = json->GetString("playerName");
	playerBboxName = json->GetString("playerBboxName");
	myBboxName = json->GetString("myBboxName");
	health = json->GetInt("health", health);
}

void EnemyControllerScript::TakeDamage(unsigned damage)
{
	if (health - damage < 0)
	{
		health = 0;
		gameobject->SetActive(false);
	}
	else
	{
		health -= damage;
	}
}

inline math::float3 EnemyControllerScript::GetPosition() const
{
	assert(gameobject->transform != nullptr);
	return gameobject->transform->GetGlobalPosition();
}

inline math::float3 EnemyControllerScript::GetPlayerPosition() const
{
	assert(player->transform != nullptr);
	return player->transform->GetGlobalPosition();
}

inline float EnemyControllerScript::GetDistanceTo(math::float3& position) const
{
	math::float3 enemyDistance = GetPosition();
	return enemyDistance.Distance(position);
}

inline float EnemyControllerScript::GetDistanceTo2D(math::float3& position) const
{
	math::float3 enemyDistance = GetPosition();
	enemyDistance.y = position.y;
	return enemyDistance.Distance(position);
}

inline bool EnemyControllerScript::IsCollidingWithPlayer() const
{
	assert(myBbox != nullptr && playerBbox != nullptr);
	return myBbox->Intersects(*playerBbox);
}

void EnemyControllerScript::Move(float speed, math::float3& direction) const
{
	math::float3 movement = direction.Normalized() * speed * App->time->gameDeltaTime;
	gameobject->transform->SetPosition(gameobject->transform->GetPosition() + movement);
}

void EnemyControllerScript::MoveTowards(float speed) const
{
	math::float3 movement = gameobject->transform->front.Normalized() * -speed * App->time->gameDeltaTime;
	gameobject->transform->SetPosition(gameobject->transform->GetPosition() + movement);
}

void EnemyControllerScript::LookAt2D(math::float3& position)
{
	math::float3 auxPos = position;
	auxPos.y = GetPosition().y;
	gameobject->transform->LookAt(auxPos);
}