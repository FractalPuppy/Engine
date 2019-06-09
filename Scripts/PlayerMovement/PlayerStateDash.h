#ifndef __PLAYERSTATEDASH_H_
#define __PLAYERSTATEDASH_H_

#include "PlayerState.h"
#include "Math/float3.h"
#include <vector>

class PlayerMovement;

class PlayerStateDash :
	public PlayerState
{
public:
	PlayerStateDash(PlayerMovement * PM, const char* trigger,
		math::float3 boxSize = math::float3(500.f, 500.f, 500.f));
	~PlayerStateDash();

	void Update() override;
	void Enter() override;
	void CheckInput() override;

public:
	math::float3 intersectionPoint;
	std::vector<float3>path;
	unsigned pathIndex = 0;
	float dashSpeed = 900.0f;
	float dashPreparationTime = 0.1f;

	GameObject* dashFX = nullptr;
	GameObject* dashMesh = nullptr;
	math::float3 meshOriginalScale = math::float3::one;

	float originalScalator = 1.05f;
	float scalator = originalScalator;
	float scalatorDecay = 0.006f;
};

#endif // __PLAYERSTATEDASH_H_