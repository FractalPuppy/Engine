#ifndef  __BossProjectileScript_h__
#define  __BossProjectileScript_h__

#include "BaseScript.h"
#include "Math/float3.h"

#ifdef BossProjectileScript_EXPORTS
#define BossProjectileScript_API __declspec(dllexport)
#else
#define BossProjectileScript_API __declspec(dllimport)
#endif

class ComponentRenderer;
class ComponentBoxTrigger;
class PlayerMovement;

class BossProjectileScript_API BossProjectileScript : public Script
{
public:
	void Awake() override;
	void Start() override;
	void Update() override;
	void OnTriggerEnter(GameObject* go) override;
	void Expose(ImGuiContext* context) override;

	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;

	inline virtual BossProjectileScript* Clone() const
	{
		return new BossProjectileScript(*this);
	}
private:
	
	GameObject* playerGO = nullptr;
	PlayerMovement* playerScript = nullptr;

	ComponentRenderer* render = nullptr;
	ComponentBoxTrigger* trigger = nullptr;

	float waitingTime = 0.8f;
	float waitTimer = 0.0f;
	float damageAmount = 5.0f;
	float projectileSpeed = 1.0f;
	float projectileLife = 5.0f;
	float shotTimer = 0.0f;
	float acceleration = 10.0f;
	float currentSpeed = 0.0f;

	bool fadeInComplete = false;
	bool waitingComplete = false;
	bool hasDamaged = false;
	

	math::float3 targetPosition = math::float3::zero;
	math::float3 shootingDirection = math::float3::zero;
};

#endif __BossProjectileScript_h__
