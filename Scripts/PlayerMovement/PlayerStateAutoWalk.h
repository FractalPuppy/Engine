#ifndef __PLAYERSTATEAUTOWALK_H_
#define __PLAYERSTATEAUTOWALK_H_

#include "PlayerState.h"

class PlayerStateAutoWalk : public PlayerState
{
public:
	PlayerStateAutoWalk(PlayerMovement * PM, const char * trigger);
	~PlayerStateAutoWalk();

	void Update() override;
	void Enter() override;

	void SetWalkPosition(math::float3 walkPos);
	void SetPlayerCameraPosition(math::float3 newPosition);
	void SetPlayerCameraRotation(math::Quat newRotation);

	float CalculateCameraLambda();

	math::float3 InterpolateFloat3(const math::float3 first, const math::float3 second, float lambda);
	math::Quat InterpolateQuat(const math::Quat first, const math::Quat second, float lambda);

public:

	std::vector<float3>path;
	unsigned pathIndex = 0u;

	math::float3 cameraNewPosition = math::float3(-6093.5f, 922.2f, -6312.7f);
	math::float3 cameraNewRotationEuler = math::float3(152.2f, 0.f, -180.f);
	math::Quat cameraNewRotation = math::Quat::identity;

private:

	math::float3 walkPosition = math::float3::zero;
	math::float3 cameraResetPosition = math::float3::zero;

	math::Quat cameraResetRotation = math::Quat::identity;

	float defaultMaxDist = 10000.f;
	float cameraMovementTimer = 0.0f;

	float firstLambda = 0.0f;
	float cameraMovementDuration = 10.0f;
};

#endif // __PLAYERSTATEAUTOWALK_H_