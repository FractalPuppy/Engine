#include "CameraController.h"
#include "ModuleTime.h"
#include "Application.h"
#include "ModuleScene.h"
#include "ModuleTime.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "imgui.h"
#include "JSON.h"
#include "Math/MathFunc.h"

#define ANGLE_MULTIPLIER 0.05f

CameraController_API Script* CreateScript()
{
	CameraController* instance = new CameraController;
	return instance;
}

void CameraController::Start()
{
	player = App->scene->FindGameObjectByName("Player");
	enemyPos = App->scene->FindGameObjectByName("BasicEnemy")->transform->GetPosition();
	assert(player != nullptr);
	offset = gameobject->transform->GetPosition() - player->transform->GetPosition();
	math::float3 newPosition = offset + player->transform->GetPosition();
	gameobject->transform->SetPosition(newPosition);
	

	//Shake(10.0f, 30.0f, 0.25f, 0.75f);
}

void CameraController::Update()
{
	math::float3 newPosition = offset + player->transform->GetPosition();
	if (isShaking)
	{
		ShakeCamera(newPosition);
	}
	gameobject->transform->SetPosition(newPosition);
}

void CameraController::Shake(float duration, float intensity, float fadeInTime, float fadeOutTime, bool linearfade)
{
	shakeDuration = duration;
	shakeIntensity = intensity;

	shakeFadeInTime = fadeInTime;
	shakeFadeOutTime = fadeOutTime;
	shakelinearFade = linearfade;

	roll = 0.0f;

	if (!isShaking)
	{
		isShaking = true;
		originalRotation = gameobject->transform->GetRotation();
		shakeTimer = .0f;
	}
}

void CameraController::ShakeCamera(math::float3& position)
{
	shakeTimer += App->time->gameDeltaTime;
	float range = shakeIntensity;

	if (shakeTimer >= shakeDuration)
	{
		isShaking = false;
		gameobject->transform->SetRotation(originalRotation);
	}
	else
	{
		if (shakeTimer <= shakeFadeInTime * shakeDuration)
		{
			range = Lerp(0, range, shakeTimer / (shakeFadeInTime * shakeDuration));
			if (!shakelinearFade)
			{
				range = shakeIntensity * SmoothStep(0, shakeIntensity, range);
			}
		}
		else if (shakeTimer >= shakeFadeOutTime * shakeDuration)
		{
			float fadeOutTime = (1 - shakeFadeOutTime) * shakeDuration;
			range = Lerp(range, 0, (shakeTimer - shakeFadeOutTime * shakeDuration) / fadeOutTime);
			if (!shakelinearFade)
			{
				range = shakeIntensity * SmoothStep(0, shakeIntensity, range);
			}
		}

		math::float3 lastPosition = position;
		position = math::float3::RandomSphere(rand, position, range);
		position = (position + lastPosition) * 0.5f;

		float lastRoll = roll;
		roll = ANGLE_MULTIPLIER * range * (rand.Float() * 2 - 1);
		roll = (lastRoll + roll) * 0.5f;
		gameobject->transform->SetRotation(originalRotation.Mul(Quat::RotateZ(math::DegToRad(roll))));
	}
}

void CameraController::zoom(math::float3 newPosition)
{
	float time = App->time->realDeltaTime;
	math::float3 move = math::float3((gameobject->transform->GetPosition().x - enemyPos.x) / 150, (gameobject->transform->GetPosition().x - 1000) / 150, (gameobject->transform->GetPosition().z - enemyPos.z) / 150);
	while (App->time->realDeltaTime <= (time + 150))
	{
		newPosition = newPosition += move;
		gameobject->transform->SetPosition(newPosition);
	}
	while (App->time->realDeltaTime <= (time + 300))
	{
		newPosition = newPosition -= move;
		gameobject->transform->SetPosition(newPosition);
	}
}