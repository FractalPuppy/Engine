#ifndef __StompSkill_h_
#define __StompSkill_h_

#include "MeleeSkill.h"

#include "Math/float3.h"
#include <vector>

class GameObject;

class StompSkill :
	public MeleeSkill
{
public:
	StompSkill(PlayerMovement* PM, const char* trigger, ComponentBoxTrigger* attackBox);
	~StompSkill();


	void Start() override;
	void UseSkill() override;

private: 
	void CheckInput() override;

private:
	math::float3 intersectionPoint;
	GameObject* StompFX = nullptr;

	float range = 1000.f;
};

#endif // __StompSkill_h_