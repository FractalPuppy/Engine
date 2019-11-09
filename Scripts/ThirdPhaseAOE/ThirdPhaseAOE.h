#ifndef  __ThirdPhaseAOE_h__
#define  __ThirdPhaseAOE_h__

#include "BaseScript.h"

#ifdef ThirdPhaseAOE_EXPORTS
#define ThirdPhaseAOE_API __declspec(dllexport)
#else
#define ThirdPhaseAOE_API __declspec(dllimport)
#endif

class ComponentBoxTrigger;
class PlayerMovement;
class BossBehaviourScript;
class CameraController;

class ThirdPhaseAOE_API ThirdPhaseAOE : public Script
{
	void Awake() override;
	void Start() override;
	void Update() override;
	void Expose(ImGuiContext* context) override;
	void OnTriggerEnter(GameObject* go) override;
	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;

	inline virtual ThirdPhaseAOE* Clone() const
	{
		return new ThirdPhaseAOE(*this);
	}
public:
	int aoeType = 0;
	int repetitions = 10;
	bool hasDamaged = false;
	float damageFirst = 25.0f;
	float damageSecond = 15.0f;

	void HandleFirstAOE();
	void HandleReplicas();

	//First AOE variables
	float timeToExplodeFirstAOE = 1.5f;
	float timeToFinishFirstAOE = 3.0f;
	float firstAOEtimer = 0.0f;
	bool firstExploded = false;
	GameObject* prepParticlesFirstAOEGO = nullptr;
	GameObject* hitParticlesFirstAOEGO = nullptr;
	GameObject* smashParticlesFirst = nullptr;
	GameObject* boxTriggerFirstAOEGO = nullptr;
	GameObject* rockParticlesFirstAOEGO = nullptr;
	ComponentBoxTrigger* boxTriggerComponentFirst = nullptr;

	//second AOE variables
	float timeToSpawnRepetition = 0.7f;
	float timeToFinishRepetition = 1.5f;
	float secondAOEtimer = 0.0f;
	float secondExploded = false;
	GameObject* prepParticlesSecondAOEGO = nullptr;
	GameObject* boxTriggerSecondAOEGO = nullptr;
	ComponentBoxTrigger* boxTriggerComponentSecond = nullptr;

	GameObject* playerGO = nullptr;
	PlayerMovement* playerScript = nullptr;

	GameObject* bossGO = nullptr;
	BossBehaviourScript* BossScript = nullptr;

	GameObject* playerCamera = nullptr;
	CameraController* cameraScript = nullptr;

	bool particlesSet = false;
	bool particlesReplica = false;
	bool replicaSpawned = false;
};

#endif __ThirdPhaseAOE_h__
