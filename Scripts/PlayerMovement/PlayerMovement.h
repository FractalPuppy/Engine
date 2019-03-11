#ifndef  __PlayerMovement_h__
#define  __PlayerMovement_h__

class GameObject;
class JSON_value;
struct ImGuiContext;
#include "BaseScript.h"
#include "Math/float3.h"

#ifdef PlayerMovement_EXPORTS
#define PlayerMovement_API __declspec(dllexport)

#else
#define PlayerMovement_API __declspec(dllimport)
#endif

class PlayerMovement_API PlayerMovement : public Script
{
public:
	void Expose(ImGuiContext* context) override;

	void Start() override;
	void Update() override;
	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;

private:
	float speed = 2.0f;
	float3 a = float3(0, 0, 0); //TODO ZERO
};

extern "C" PlayerMovement_API PlayerMovement* CreateScript();

#endif __PlayerMovement_h__
