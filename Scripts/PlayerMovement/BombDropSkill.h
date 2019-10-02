#ifndef  __BombDropSkill_h__
#define  __BombDropSkill_h__

#include "MeleeSkill.h"

#include "Math/float3.h"
#include <vector>

class GameObject;

class BombDropSkill :
	public MeleeSkill
{
public:
	BombDropSkill(PlayerMovement* PM, const char* trigger, ComponentBoxTrigger* attackBox);
	~BombDropSkill();

	void Start() override;
	void UseSkill() override;

private:
	void CheckInput() override;

private:
	float bombDropMaxDistance = 10000.0f;
	float bombDropSpeed = 900.0f;
	float bombDropPreparationTime = 0.1f;
	math::float3 intersectionPoint;
	GameObject* bombDropFX = nullptr;
	std::vector<float3>path;
	unsigned pathIndex = 0;
};
#endif __BombDropSkill_h__