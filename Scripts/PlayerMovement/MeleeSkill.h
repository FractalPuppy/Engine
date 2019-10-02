#ifndef  __MeleeSkill_h__
#define  __MeleeSkill_h__

#include "BasicSkill.h"

#include "Math/float3.h"

class PlayerMovement;
class ComponentBoxTrigger;
class ImGuiContext;
class JSON_value;

class MeleeSkill :
	public BasicSkill
{
public:
	MeleeSkill(PlayerMovement* PM, const char* trigger, ComponentBoxTrigger* attackBox);
	virtual ~MeleeSkill();

	void Update() override;

	void Expose(ImGuiContext* context) override;
	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;

protected:
	virtual void Reset();

protected:
	ComponentBoxTrigger* attackBoxTrigger = nullptr;

	float hitDelay = 0.0f; // Time to delay the hitbox enable

	math::float3 boxSize = math::float3::zero;
	math::float3 boxPosition = math::float3::zero;
};

#endif __MeleeSkill_h__