#ifndef  __RainSkill_h__
#define  __RainSkill_h__

#include "RangeSkill.h"
#include <vector>
#include "Math/float4.h"

#define AREA_SIZE 200.f
#define VERTICAL_OFFSET 300.f

class GameObject;
class ComponentRenderer;
class ComponentBoxTrigger;
class ResourceMaterial;

class RainSkill :
	public RangeSkill
{
public:

	struct MacheteUnit 
	{
		GameObject* machete = nullptr;
		ComponentRenderer* renderer = nullptr;
		ComponentBoxTrigger* trigger = nullptr;
		math::float3 originalScale;
		bool landed = false;
		unsigned colliderFrames = 5u;
	};
	RainSkill(PlayerMovement* PM, const char* trigger, const char* prefab);
	~RainSkill();

	void Start() override;
	void Prepare() override;
	bool OnCancel() override;
	std::vector<MacheteUnit> machetes;

	bool canBeLaunched = false;
	float targetHeight = 0.f;
	math::float4 decalOriginalColor = float4::one;

	ResourceMaterial* decalMaterial = nullptr;
};

#endif