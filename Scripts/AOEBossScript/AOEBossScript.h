#ifndef  __AOEBossScript_h__
#define  __AOEBossScript_h__

#include "BaseScript.h"
#include <vector>
#include "Math/float3.h"



class ComponentBoxTrigger;
class PlayerMovement;

#ifdef AOEBossScript_EXPORTS
#define AOEBossScript_API __declspec(dllexport)
#else
#define AOEBossScript_API __declspec(dllimport)
#endif

class AOEBossScript_API AOEBossScript : public Script
{
public:
	void Awake() override;
	void Start() override;
	void Update() override;
	void OnTriggerEnter(GameObject* go) override;
	void Expose(ImGuiContext* context) override;

	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;

	inline virtual AOEBossScript* Clone() const
	{
		return new AOEBossScript(*this);
	}

	int circleType = 1;
private:

	GameObject* prepParticlesGO = nullptr;
	GameObject* beamParticlesGO = nullptr;
	GameObject* boxTriggerGO = nullptr;

	ComponentBoxTrigger* boxTrigger = nullptr;

	GameObject* playerGO = nullptr;
	PlayerMovement* playerScript = nullptr;

	

	float duration = 3.0f;
	float timerFade = 2.0f;
	float timer = 0.0f;
	float damageToPlayer = 5.0f;

	bool hasDamaged = false;

	float spawnRadius = 0.0f;

	void SpawnSecondaryCircles();
};

extern "C" AOEBossScript_API Script* CreateScript();

#endif __AOEBossScript_h__
