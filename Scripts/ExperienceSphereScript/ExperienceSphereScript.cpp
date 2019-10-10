#include "ExperienceSphereScript.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleTime.h"

#include "GameObject.h"
#include "ComponentTransform.h"

#include "ExperienceController.h"

#include "imgui.h"
#include "JSON.h"

ExperienceSphereScript_API Script* CreateScript()
{
	ExperienceSphereScript* instance = new ExperienceSphereScript;
	return instance;
}

void ExperienceSphereScript::Start()
{
	targetGO = App->scene->FindGameObjectByTag(targetTag.c_str());

	if (targetGO == nullptr)
	{
		LOG("Target GO with tag '%s' couldn't be found", targetTag.c_str());
		gameobject->deleteFlag = true;
	}

	GameObject* xpGO = App->scene->FindGameObjectByName("Xp");
	if (xpGO == nullptr)
	{
		LOG("Xp controller GO couldn't be found \n");
	}
	else
	{
		experienceController = xpGO->GetComponent<ExperienceController>();
		if (experienceController == nullptr)
		{
			LOG("experienceController couldn't be found \n");
		}
	}
}

void ExperienceSphereScript::Update()
{
	math::float3 targetPosition = targetGO->transform->position + offset;
	math::float3 myPosition = gameobject->transform->position;

	float distance = myPosition.Distance(targetPosition);

	// If is near enough pick up else chase
	if (distance > 10.0f)
	{
		timer += App->time->gameDeltaTime;

		// Look at target
		gameobject->transform->LookAt(targetPosition);

		// Move towards target
		math::float3 movement = gameobject->transform->front.Normalized() * -speed * App->time->gameDeltaTime * timer;
		gameobject->transform->SetPosition(myPosition + movement);
	}
	else
	{
		// Give experience to player
		if (experienceController != nullptr)
			experienceController->AddXP(experience);

		timer = 0.0f;
		gameobject->deleteFlag = true;
	}
}

void ExperienceSphereScript::Expose(ImGuiContext* context)
{
	char* goName = new char[64];
	strcpy_s(goName, strlen(targetTag.c_str()) + 1, targetTag.c_str());
	ImGui::InputText("playerTag", goName, 64);
	targetTag = goName;
	delete[] goName;

	ImGui::DragFloat("Speed", &speed);
	ImGui::DragFloat3("Offset", (float*)&offset);
}

void ExperienceSphereScript::Serialize(JSON_value* json) const
{
	assert(json != nullptr);
	json->AddString("targetTag", targetTag.c_str());
	json->AddFloat("speed", speed);
	json->AddFloat3("offset", offset);
}

void ExperienceSphereScript::DeSerialize(JSON_value* json)
{
	assert(json != nullptr);
	targetTag = json->GetString("targetTag", "Player");
	speed = json->GetFloat("speed", 550.0f);
	offset = json->GetFloat3("offset");
}