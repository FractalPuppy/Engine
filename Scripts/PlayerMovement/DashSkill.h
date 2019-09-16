#ifndef  __DashSkill_h__
#define  __DashSkill_h__

#include "MeleeSkill.h"

#include "Math/float3.h"
#include <vector>

class GameObject;
class ComponentRenderer;

class DashSkill :
	public MeleeSkill
{
public:
	DashSkill(PlayerMovement* PM, const char* trigger, ComponentBoxTrigger* attackBox);
	~DashSkill();

	void Start() override;

protected:
	void UseSkill() override;
	void Reset() override;

	void CheckInput() override;

public:
	math::float3 intersectionPoint;
	std::vector<float3>path;
	unsigned pathIndex = 0;
	float dashSpeed = 1500.0f;
	float dashPreparationTime = 0.05f;

	GameObject* dashFX = nullptr;
	GameObject* dashMesh = nullptr;
	ComponentRenderer* playerRenderer = nullptr;

	math::float3 meshOriginalScale = math::float3::one;


	float originalScalator = 1.05f;
	float scalator = originalScalator;
	float scalatorDecay = 0.006f;
};

#endif __DashSkill_h__