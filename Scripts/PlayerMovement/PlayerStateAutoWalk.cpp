#include "PlayerMovement.h"

#include "ModuleNavigation.h"
#include "ModuleTime.h"

#include "GameObject.h"
#include "ComponentTransform.h"
#include "CameraController.h"

#include "PlayerStateAutoWalk.h"

#include "Math/MathFunc.h"

PlayerStateAutoWalk::PlayerStateAutoWalk(PlayerMovement * PM, const char * trigger) :
	PlayerState(PM, trigger)
{
}


PlayerStateAutoWalk::~PlayerStateAutoWalk()
{
}

void PlayerStateAutoWalk::Update()
{
	if (walkPosition != math::float3::zero)
	{
		math::float3 correctionPos(0.f, player->OutOfMeshCorrectionY, 0.f);
		if (player->App->navigation->FindPath(player->gameobject->transform->position, walkPosition,
			path, PathFindType::FOLLOW, correctionPos, defaultMaxDist, player->straightPathingDistance))
		{
			//case the player clicks outside of the floor mesh but we want to get close to the floors edge
			pathIndex = 0;
		}
	}

	if (path.size() > 0)
	{
		math::float3 currentPosition = player->gameobject->transform->GetPosition();
		while (pathIndex < path.size() && currentPosition.DistanceSq(path[pathIndex]) < MINIMUM_PATH_DISTANCE)
		{
			pathIndex++;
		}
		if (pathIndex < path.size())
		{
			player->gameobject->transform->LookAt(path[pathIndex]);
			math::float3 direction = (path[pathIndex] - currentPosition).Normalized();
			//lerping if necessary
			lerpCalculations(direction, -player->gameobject->transform->front, path[pathIndex]);

			math::float3 finalWalkingSpeed = player->walkingSpeed * direction * player->App->time->gameDeltaTime;
			finalWalkingSpeed *= (1 + (player->GetTotalPlayerStats()->dexterity * 0.005f));
			player->gameobject->transform->SetPosition(currentPosition + finalWalkingSpeed);
			playerWalking = true;
			playerWalkingToHit = true;
		}

		else
		{
			playerWalking = false;
			return;
		}
	}
	//no path, stop
	else
	{
		playerWalking = false;
		return;
	}

	//Camera movement
	cameraMovementTimer += player->App->time->gameDeltaTime;

	firstLambda = CalculateCameraLambda();
	SetPlayerCameraPosition(InterpolateFloat3(cameraResetPosition, cameraNewPosition, firstLambda));
	SetPlayerCameraRotation(InterpolateQuat(cameraResetRotation, cameraNewRotation, firstLambda));

}

void PlayerStateAutoWalk::Enter()
{
	//Deactivate camera script
	player->playerCamera->GetComponent<CameraController>()->Enable(false);

	cameraNewRotation = cameraNewRotation.FromEulerXYZ(math::DegToRad(cameraNewRotationEuler.x),
		math::DegToRad(cameraNewRotationEuler.y), math::DegToRad(cameraNewRotationEuler.z));

	cameraResetPosition = player->playerCamera->transform->GetPosition();
	cameraResetRotation = player->playerCamera->transform->GetRotation();
}

void PlayerStateAutoWalk::SetWalkPosition(math::float3 walkPos)
{
	walkPosition = walkPos;
}

void PlayerStateAutoWalk::SetPlayerCameraPosition(math::float3 newPosition)
{
	player->playerCamera->transform->SetPosition(newPosition);
}

void PlayerStateAutoWalk::SetPlayerCameraRotation(math::Quat newRotation)
{
	player->playerCamera->transform->SetRotation(newRotation);
}

float PlayerStateAutoWalk::CalculateCameraLambda()
{
	return (cameraMovementTimer + player->App->time->gameDeltaTime) / cameraMovementDuration;
}

math::float3 PlayerStateAutoWalk::InterpolateFloat3(const math::float3 first, const math::float3 second, float lambda)
{
	return first * (1.0f - lambda) + second * lambda;
}

math::Quat PlayerStateAutoWalk::InterpolateQuat(const math::Quat first, const math::Quat second, float lambda)
{
	math::Quat result;
	float dot = first.Dot(second);

	if (dot >= 0.0f) // Interpolate through the shortest path
	{
		result.x = first.x*(1.0f - lambda) + second.x*lambda;
		result.y = first.y*(1.0f - lambda) + second.y*lambda;
		result.z = first.z*(1.0f - lambda) + second.z*lambda;
		result.w = first.w*(1.0f - lambda) + second.w*lambda;
	}
	else
	{
		result.x = first.x*(1.0f - lambda) - second.x*lambda;
		result.y = first.y*(1.0f - lambda) - second.y*lambda;
		result.z = first.z*(1.0f - lambda) - second.z*lambda;
		result.w = first.w*(1.0f - lambda) - second.w*lambda;
	}

	result.Normalize();

	return result;
}
