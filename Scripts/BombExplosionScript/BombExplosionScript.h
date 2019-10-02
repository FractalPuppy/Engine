#ifndef  __BombExplosionScript_h__
#define  __BombExplosionScript_h__

#include "BaseScript.h"

#ifdef BombExplosionScript_EXPORTS
#define BombExplosionScript_API __declspec(dllexport)
#else
#define BombExplosionScript_API __declspec(dllimport)
#endif

enum class ExplosionState
{
	None,
	Appear,
	Grow,
	Explode,
	Finished
};

class ComponentRenderer;
class GameObject;
class PlayerMovement;

class BombExplosionScript_API BombExplosionScript : public Script
{
	void Awake() override;
	void Start() override;
	void Update() override;
	void OnTriggerEnter(GameObject* go) override;
	void Expose(ImGuiContext* context) override;

	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;

	inline virtual BombExplosionScript* Clone() const
	{
		return new BombExplosionScript(*this);
	}
private:
	ExplosionState currentState = ExplosionState::None;

	ComponentTransform* myTransform = nullptr;
	ComponentRenderer* myRenderer = nullptr;
	GameObject* renderGO = nullptr;
	GameObject* hitboxGO = nullptr;
	GameObject* particlesGO = nullptr;
	GameObject* playerGO = nullptr;

	PlayerMovement* playerScript = nullptr;

private:

	float damageToPlayer = 10.0f;
	float dissolveSpeed = 1.0f;
	float scalingSpeed = 0.002f;
	float finalScale = 2.0f;
	bool hasDamaged = false;
};

#endif __BombExplosionScript_h__
