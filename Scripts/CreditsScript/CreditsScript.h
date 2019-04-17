#ifndef  __CreditsScript_h__
#define  __CreditsScript_h__

#include "BaseScript.h"

#include "Math/float2.h"

#ifdef CreditsScript_EXPORTS
#define CreditsScript_API __declspec(dllexport)
#else
#define CreditsScript_API __declspec(dllimport)
#endif

class ComponentTransform2D;

class CreditsScript_API CreditsScript : public Script
{
public:
	void Expose(ImGuiContext* context) override;

	void Start() override;
	void Update() override;
	void ResetScript();


	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;
public:
	bool creditsDone = false;
	ComponentTransform2D* transform2D = nullptr;
	math::float2 initialPosition = math::float2::zero;

private:
	float speed = 100.0f;
	float stopValue = 900.0f;
};
#endif __CreditsScript_h__
