#include "ScaleOverTime.h"

#include "Application.h"
#include "ModuleTime.h"

#include "GameObject.h"
#include "ComponentTransform.h"

#include "imgui.h"
#include "JSON.h"
#include <assert.h>

ScaleOverTime_API Script* CreateScript()
{
	ScaleOverTime* instance = new ScaleOverTime;
	return instance;
}

void ScaleOverTime::Start()
{
	transform = gameobject->GetComponent<ComponentTransform>();
	if (transform == nullptr)
	{
		LOG("Error: Component transform couldn't be found on %s.", gameobject->name.c_str());
	}
	else
	{
		startScale = transform->scale;
	}

	assert(transform != nullptr);
}

void ScaleOverTime::Update()
{
	if (acceleration)
	{
		timerX += App->time->gameDeltaTime;
		timerY += App->time->gameDeltaTime;
		timerZ += App->time->gameDeltaTime;
	}
	else
	{
		timerX = App->time->gameDeltaTime;
		timerY = App->time->gameDeltaTime;
		timerZ = App->time->gameDeltaTime;
	}

	if (pingPong)
		ComputeScalePingPong();
	else
		ComputeScale();
}

void ScaleOverTime::Expose(ImGuiContext* context)
{
	ImGui::DragFloat("Speed", &speed, 0.01f);
	ImGui::DragFloat3("Final Scale", (float*)&finalScale);
	ImGui::Checkbox("Loop", &loop);
	if (loop)
	{
		ImGui::Checkbox("Ping-Pong", &pingPong);
	}

	if (ImGui::Checkbox("Acceleration", &acceleration))
	{
		timerX = 0.0f;
		timerY = 0.0f;
		timerZ = 0.0f;
	}
}

void ScaleOverTime::Serialize(JSON_value* json) const
{
	assert(json != nullptr);
	json->AddFloat("speed", speed);
	json->AddFloat3("finalScale", finalScale);
	json->AddInt("loop", loop);
	json->AddInt("pingPong", pingPong);
	json->AddInt("acceleration", acceleration);
}

void ScaleOverTime::DeSerialize(JSON_value* json)
{
	assert(json != nullptr);
	speed = json->GetFloat("speed", 0.1f);
	finalScale = json->GetFloat3("finalScale");
	loop = json->GetInt("loop");
	pingPong = json->GetInt("pingPong");
	acceleration = json->GetInt("acceleration", 1);
}

void ScaleOverTime::ComputeScale()
{
	math::float3 newScale = transform->scale;

	// X
	if (transform->scale.x < finalScale.x) newScale.x += speed * timerX;	// Increase scale
	else if (loop) { newScale.x = startScale.x; timerX = 0.0f; }			// Loop: reset scale and timer
	else newScale.x = finalScale.x;											// No loop: set final scale

	// Y
	if (transform->scale.y < finalScale.y) newScale.y += speed * timerY;	// Increase scale
	else if (loop) {newScale.y = startScale.y; timerY = 0.0f;}				// Loop: reset scale and timer
	else newScale.y = finalScale.y;											// No loop: set final scale

	// Z
	if (transform->scale.z < finalScale.z) newScale.z += speed * timerZ;	// Increase scale
	else if (loop) {newScale.z = startScale.z; timerZ = 0.0f;}				// Loop: reset scale and timer
	else newScale.z = finalScale.z;											// No loop: set final scale

	// Apply new scale. If final scale reached and no loop disable script
	if (finalScale != newScale)
		transform->SetScale(newScale);
	else
		this->Enable(false);
}

void ScaleOverTime::ComputeScalePingPong()
{
	math::float3 newScale = transform->scale;

	// X
	if (!reverse[0])
	{
		if (transform->scale.x < finalScale.x) newScale.x += speed * timerX;	// Increase scale
		else { timerX = 0.0f; reverse[0] = true;}								// PingPong Loop: reset timer and set reverse
	}
	else
	{
		if (transform->scale.x > startScale.x) newScale.x -= speed * timerX;	// Decrease scale
		else { timerX = 0.0f; reverse[0] = false;}								// PingPong Loop: reset timer and set !reverse
	}
	

	// Y
	if (!reverse[1])
	{
		if (transform->scale.y < finalScale.y) newScale.y += speed * timerY;	// Increase scale
		else { timerY = 0.0f; reverse[1] = true;}								// PingPong Loop: reset timer and set reverse
	}
	else
	{
		if (transform->scale.y > startScale.y) newScale.y -= speed * timerY;	// Decrease scale
		else { timerY = 0.0f; reverse[1] = false;}								// PingPong Loop: reset timer and set !reverse
	}

	// Z
	if (!reverse[2])
	{
		if (transform->scale.z < finalScale.z) newScale.z += speed * timerZ;	// Increase scale
		else { timerZ = 0.0f; reverse[2] = true;}								// PingPong Loop: reset timer and set reverse
	}
	else
	{
		if (transform->scale.z > startScale.z) newScale.z -= speed * timerZ;	// Decrease scale
		else { timerZ = 0.0f; reverse[2] = false;}								// PingPong Loop: reset timer and set !reverse
	}

	transform->SetScale(newScale);	// Apply new scale
}