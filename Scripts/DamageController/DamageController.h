#ifndef  __DamageController_h__
#define  __DamageController_h__

#include "BaseScript.h"

#ifdef DamageController_EXPORTS
#define DamageController_API __declspec(dllexport)
#else
#define DamageController_API __declspec(dllimport)
#endif

#include "Damage.h"

class GameObject;
class ComponentTransform;

class DamageController_API DamageController : public Script
{
public:
	void Start() override;
	void Update() override;

	bool AddDamage(ComponentTransform* position, int damage, int type);

private:
	GameObject* uiDamage[10];
	Damage damageList[10];
};

#endif __DamageController_h__
