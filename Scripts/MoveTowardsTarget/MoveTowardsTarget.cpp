#include "MoveTowardsTarget.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleTime.h"

#include "GameObject.h"
#include "ComponentTransform.h"

#include "imgui.h"
#include "JSON.h"

MoveTowardsTarget_API Script* CreateScript()
{
	MoveTowardsTarget* instance = new MoveTowardsTarget;
	return instance;
}

void MoveTowardsTarget::Start()
{
	targetGO = App->scene->FindGameObjectByTag(targetTag.c_str());

	if (targetGO == nullptr)
	{
		LOG("Target GO with tag '%s' couldn't be found", targetTag.c_str());
		gameobject->deleteFlag = true;
	}
}

void MoveTowardsTarget::Update()
{
	math::float3 targetPosition = targetGO->transform->position;
	math::float3 myPosition = gameobject->transform->position;

	float distance = myPosition.Distance(targetPosition);

	if (distance > 0.5f)
	{
		// Look at target
		gameobject->transform->LookAt(targetPosition);

		// Move towards target
		math::float3 movement = gameobject->transform->front.Normalized() * -speed * App->time->gameDeltaTime;
		gameobject->transform->SetPosition(myPosition + movement);
	}
	else
	{
		gameobject->deleteFlag = true;
	}
}

void MoveTowardsTarget::Expose(ImGuiContext* context)
{
	ImGui::DragFloat("Speed", &speed);
}

void MoveTowardsTarget::Serialize(JSON_value* json) const
{
	assert(json != nullptr);
	json->AddString("targetTag", targetTag.c_str());
	json->AddFloat("speed", speed);
}

void MoveTowardsTarget::DeSerialize(JSON_value* json)
{
	assert(json != nullptr);
	targetTag = json->GetString("targetTag", "Player");
	speed = json->GetFloat("speed", 100.0f);
}
