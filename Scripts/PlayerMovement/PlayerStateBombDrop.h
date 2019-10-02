#ifndef __PLAYERSTATEBOMBDROP_H_
#define __PLAYERSTATEBOMBDROP_H_

#include "PlayerState.h"
#include "Math/float3.h"
#include <vector>

class PlayerStateBombDrop :
	public PlayerState
{
public:

	PlayerStateBombDrop(PlayerMovement* PM, const char* trigger,
		math::float3 boxSize = math::float3(500.f, 500.f, 500.f));
	~PlayerStateBombDrop();

	void Update() override;
	void Enter() override;
	void CheckInput() override;

public:

	float bombDropSpeed = 900.0f;
	float bombDropPreparationTime = 0.1f;
	math::float3 intersectionPoint;
	GameObject* bombDropFX = nullptr;
	std::vector<float3>path;
	unsigned pathIndex = 0;
};

#endif