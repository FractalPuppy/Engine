#ifndef  __IntroScript_h__
#define  __IntroScript_h__

class JSON_value;
#include "BaseScript.h"
#include "Math/float3.h"

#ifdef IntroScript_EXPORTS
#define IntroScript_API __declspec(dllexport)
#else
#define IntroScript_API __declspec(dllimport)
#endif

class IntroScript_API IntroScript : public Script
{
public:
	void Expose(ImGuiContext* context) override;

	void Start() override;
	void Update() override;

	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;
public:
	bool introDone = false;
	math::float3 initialPosition = math::float3::zero;

private:
	math::float3 distanceNormalized = math::float3::zero;
	math::float3 finalPosition = math::float3(0, 0, 1000);
	float speed = 2.0f;
};

extern "C" IntroScript_API Script* CreateScript();

#endif __IntroScript_h__
