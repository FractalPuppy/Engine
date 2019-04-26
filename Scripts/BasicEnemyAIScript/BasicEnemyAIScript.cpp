#include "BasicEnemyAIScript.h"

#include "Application.h"
#include "ModuleTime.h"
#include "ModuleScene.h"

#include "ComponentTransform.h"
#include "GameObject.h"

#include "Geometry/AABB.h"
#include "Math/float3.h"
#include "Math/Quat.h"
#include "Math/float4x4.h"
#include "imgui.h"
#include <stack>
#include "JSON.h"

BasicEnemyAIScript_API Script* CreateScript()
{
	BasicEnemyAIScript* instance = new BasicEnemyAIScript;
	return instance;
}

void BasicEnemyAIScript::Start()
{
	player = GetGameObjectRecursiveByName(App->scene->root, playerName.c_str());
	myBbox = &App->scene->FindGameObjectByName(gameObject, myBboxName.c_str())->bbox;
	playerBbox = &App->scene->FindGameObjectByName(player, playerBboxName.c_str())->bbox;
}

void BasicEnemyAIScript::Update()
{
	switch (enemyState)
	{
	case EnemyState::WAIT:
		Wait();
		break;
	case EnemyState::STANDUP:
		StandUp();
		break;
	case EnemyState::CHASE:
		Chase();
		break;
	case EnemyState::ATTACK:
		Attack();
		break;
	case EnemyState::COOLDOWN:

		break;
	case EnemyState::DEAD:
		break;
	default:
		break;
	}
}

void BasicEnemyAIScript::Expose(ImGuiContext* context)
{
	ImGui::SetCurrentContext(context);
	ImGui::InputFloat("Distance to activate", &activationDistance);
	ImGui::InputFloat("Stand-up Speed", &standupSpeed);
	ImGui::InputFloat("Y Translation", &yTranslation);

	char* bboxName = new char[64];
	strcpy_s(bboxName, strlen(myBboxName.c_str()) + 1, myBboxName.c_str());
	ImGui::InputText("My BBox Name", bboxName, 64);
	myBboxName = bboxName;
	delete[] bboxName;

	ImGui::InputFloat("Chase Speed", &chaseSpeed);

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

void BasicEnemyAIScript::Serialize(JSON_value* json) const
{
	assert(json != nullptr);
	json->AddString("playerName", playerName.c_str());
	json->AddString("playerBboxName", playerBboxName.c_str());
	json->AddString("myBboxName", myBboxName.c_str());

	//Wait variables
	json->AddFloat("activationDistance", activationDistance);

	// Stand-Up variables
	json->AddFloat("standupSpeed", standupSpeed);
	json->AddFloat("yTranslation", yTranslation);

	// Chase variables
	json->AddFloat("chaseSpeed", chaseSpeed);

}

void BasicEnemyAIScript::DeSerialize(JSON_value* json)
{
	assert(json != nullptr);
	playerName = json->GetString("playerName");
	playerBboxName = json->GetString("playerBboxName");
	myBboxName = json->GetString("myBboxName");

	//Wait variables
	activationDistance = json->GetFloat("activationDistance");

	// Stand-Up variables
	standupSpeed = json->GetFloat("standupSpeed");
	yTranslation = json->GetFloat("yTranslation");

	// Chase variables
	chaseSpeed = json->GetFloat("chaseSpeed");
}

void BasicEnemyAIScript::Wait()
{
	if (player == nullptr)
		return;

	// No animation

	float3 enemyCurrentPosition = gameObject->transform->GetGlobalPosition();
	enemyCurrentPosition.y = 0.0f;
	float3 playerCurrentPosition = player->transform->GetGlobalPosition();
	playerCurrentPosition.y = 0.0f;

	float distance = enemyCurrentPosition.Distance(playerCurrentPosition);

	if (distance < activationDistance)
	{
		enemyState = EnemyState::STANDUP;
	}
}

void BasicEnemyAIScript::StandUp()
{
	// Start playing stand-up animation

	// Translate on the Z axis
	float3 movement = gameObject->transform->up.Normalized() * standupSpeed * App->time->gameDeltaTime;
	gameObject->transform->SetPosition(gameObject->transform->GetPosition() + movement);
	auxTranslation += movement.y;

	// Check if the needed Z has been reached
	if (yTranslation <= auxTranslation)
	{
		enemyState = EnemyState::CHASE;
		auxTranslation = 0.0f;

		// Fix to avoid rotating in other axis
		gameObject->transform->position.y = player->transform->position.y;
	}
}

void BasicEnemyAIScript::Chase()
{
	if (player == nullptr)
		return;

	// Start playing run animation


	LookAtPlayer();

	// Move towards player
	math::float3 movement = gameObject->transform->front.Normalized() * -chaseSpeed * App->time->gameDeltaTime;
	gameObject->transform->SetPosition(gameObject->transform->GetPosition() + movement);

	// Check collision
	if (myBbox != nullptr && myBbox->Intersects(*playerBbox))
	{
		// Player intersected, change to attack
		enemyState = EnemyState::ATTACK;
	}
}

void BasicEnemyAIScript::Attack()
{
	// Play attack animation

	LookAtPlayer();

	if (myBbox != nullptr && !myBbox->Intersects(*playerBbox))
	{
		enemyState = EnemyState::CHASE;
	}
}

void BasicEnemyAIScript::LookAtPlayer()
{
	math::float3 enemyCurrentPosition = gameObject->transform->GetGlobalPosition();
	math::float3 playerCurrentPosition = player->transform->GetGlobalPosition();

	// Look at player
	math::float3 dir = (enemyCurrentPosition - playerCurrentPosition);
	math::Quat currentRotation = gameObject->transform->GetRotation();
	math::Quat rotation = currentRotation.LookAt(gameObject->transform->front.Normalized(), dir.Normalized(), float3::unitY, float3::unitY);
	gameObject->transform->SetRotation(rotation.Mul(currentRotation));
}

GameObject* BasicEnemyAIScript::GetGameObjectRecursiveByName(GameObject* gameObject, const char* name)
{
	if (strcmp(gameObject->name.c_str(), name) == 0)
	{
		return gameObject;
	}

	std::stack<GameObject*> stack;
	for (GameObject* child : gameObject->children)
	{
		stack.push(child);
	}

	while (!stack.empty())
	{
		GameObject* gameObject = stack.top();
		stack.pop();

		if (strcmp(gameObject->name.c_str(), name) == 0)
		{
			return gameObject;
		}

		for (GameObject* child : gameObject->children)
		{
			stack.push(child);
		}
	}
}