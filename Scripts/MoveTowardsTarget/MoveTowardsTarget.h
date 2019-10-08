#ifndef  __MoveTowardsTarget_h__
#define  __MoveTowardsTarget_h__

#include "BaseScript.h"
#include "Math/float3.h"

#ifdef MoveTowardsTarget_EXPORTS
#define MoveTowardsTarget_API __declspec(dllexport)
#else
#define MoveTowardsTarget_API __declspec(dllimport)
#endif

class GameObject;
class ExperienceController;

class MoveTowardsTarget_API MoveTowardsTarget : public Script
{
public:
	inline virtual MoveTowardsTarget* Clone() const
	{
		return new MoveTowardsTarget(*this);
	}

	void Start() override;
	void Update() override;

	void Expose(ImGuiContext* context) override;

	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;

	std::string targetTag = "Player";
	float speed = 100.0f;
	math::float3 offset;

	int experience = 0;		// Experience to give to player on pick up

private:
	GameObject* targetGO;

	// Experience script
	ExperienceController* experienceController = nullptr;
};

extern "C" MoveTowardsTarget_API Script* CreateScript();
#endif __MoveTowardsTarget_h__
