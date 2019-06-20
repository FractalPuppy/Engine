#include "CreditsScript.h"

#include "Application.h"

#include "ModuleTime.h"

#include "GameObject.h"
#include "ComponentTransform2D.h"

#include "JSON.h"
#include "imgui.h"
#include "Math/float2.h"

CreditsScript_API Script* CreateScript()
{
	CreditsScript* instance = new CreditsScript;
	return instance;
}

void CreditsScript::Expose(ImGuiContext* context)
{
	ImGui::SetCurrentContext(context);
	ImGui::InputFloat("speed", &speed);
	ImGui::InputFloat("stop value", &stopValue);
	
}

void CreditsScript::Start()
{
	int h = 0;
	transform2D = gameobject->GetComponent<Transform2D>();
	initialPosition = transform2D->getPosition();
}

void CreditsScript::Update()
{
	if (!gameobject->isActive()) { return; }

	if (transform2D != nullptr && !creditsDone)
	{
		float2 position = transform2D->getPosition();

		if ((int)position.y != (int)stopValue)
		{
			position.y += 1.0f * speed * App->time->gameDeltaTime;
			transform2D->setPosition(position);
		}
		else
		{
			creditsDone = true;
		}
	}
}

void CreditsScript_API CreditsScript::ResetScript()
{
	creditsDone = false;
	transform2D->setPosition(initialPosition);
}

void CreditsScript::Serialize(JSON_value* json) const
{
	assert(json != nullptr);
	json->AddFloat("speed", speed);
	json->AddFloat("stopValue", stopValue);
}

void CreditsScript::DeSerialize(JSON_value* json)
{
	assert(json != nullptr);
	speed = json->GetFloat("speed");
	stopValue = json->GetFloat("stopValue");
}
