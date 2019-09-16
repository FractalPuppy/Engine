#ifndef  __DamageController_h__
#define  __DamageController_h__

#include "BaseScript.h"

#ifdef DamageController_EXPORTS
#define DamageController_API __declspec(dllexport)
#else
#define DamageController_API __declspec(dllimport)
#endif

#include "Damage.h"
#include "Math/float4.h"

class GameObject;
class ComponentTransform;

class DamageController_API DamageController : public Script
{
public:
	void Start() override;
	void Update() override;

	void Expose(ImGuiContext* context) override;

	bool AddDamage(ComponentTransform* position, int damage, DamageType type);

	// Damage numbers color
	math::float4 normalColor = math::float4(1.0f, 1.0f, 1.0f, 1.0f);
	math::float4 criticalColor = math::float4(1.0f, 0.8f, 0.2f, 1.0f);
	math::float4 pollometerColor = math::float4(1.0f, 0.44f, 0.2f, 1.0f);
	math::float4 healingColor = math::float4(0.0f, 1.0f, 0.0f, 1.0f);
	math::float4 receivedColor = math::float4(1.0f, 0.0f, 0.0f, 1.0f);
	math::float4 manaColor = math::float4(0.054f, 0.549f, 0.943f, 1.0f);

private:
	GameObject* uiDamage[10];
	Damage damageList[10];
};

extern "C" DamageController_API Script* CreateScript();
#endif __DamageController_h__
