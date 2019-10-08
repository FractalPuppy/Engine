#ifndef  __ExperienceSphereScript_h__
#define  __ExperienceSphereScript_h__

#include "BaseScript.h"
#include "Math/float3.h"

#ifdef ExperienceSphereScript_EXPORTS
#define ExperienceSphereScript_API __declspec(dllexport)
#else
#define ExperienceSphereScript_API __declspec(dllimport)
#endif

class GameObject;
class ExperienceController;

class ExperienceSphereScript_API ExperienceSphereScript : public Script
{
public:
	inline virtual ExperienceSphereScript* Clone() const
	{
		return new ExperienceSphereScript(*this);
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

extern "C" ExperienceSphereScript_API Script* CreateScript();
#endif __ExperienceSphereScript_h__
