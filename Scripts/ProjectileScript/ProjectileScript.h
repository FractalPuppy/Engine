#ifndef  __ProjectileScript_h__
#define  __ProjectileScript_h__

#ifdef ProjectileScript_EXPORTS
#define ProjectileScript_API __declspec(dllexport)
#else
#define ProjectileScript_API __declspec(dllimport)
#endif

#include "BaseScript.h"

class JSON_value;
class ComponentTransform;
class ComponentBoxTrigger;
class PlayerMovement;

class ProjectileScript_API ProjectileScript : public Script
{

	void Start() override;
	void Update() override;

	void OnTriggerEnter(GameObject* go) override;

	void Expose(ImGuiContext* context) override;

	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;

public:
	float speed = 1000.0f;
	float lifeTime = 10.0f;

	std::string playerName = "Player";

private:
	ComponentTransform* transform = nullptr;
	ComponentBoxTrigger* boxTrigger = nullptr;

	GameObject* player = nullptr;
	PlayerMovement* playerScript = nullptr;

	bool shooted = false;

	float auxTimer = 0.0f;
	float activeTime = 0.0f;
};

#endif __ProjectileScript_h__
