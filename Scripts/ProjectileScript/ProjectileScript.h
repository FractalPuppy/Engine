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

	inline virtual ProjectileScript* Clone() const
	{
		return new ProjectileScript(*this);
	}

public:
	float speed = 1000.0f;
	float lifeTime = 10.0f;
	bool rotate = false;
	float rotationSpeed = 1.0f;
	bool rotationX = false;
	bool rotationY = false;
	bool rotationZ = false;
	float offsetHeight = 100.0f;


	std::string playerName = "Player";

	bool shooted = false;
	GameObject* meshGO = nullptr;
private:
	ComponentTransform* transform = nullptr;
	ComponentBoxTrigger* boxTrigger = nullptr;


	GameObject* player = nullptr;
	PlayerMovement* playerScript = nullptr;



	float auxTimer = 0.0f;
	float activeTime = 0.0f;
};

extern "C" ProjectileScript_API Script* CreateScript();

#endif __ProjectileScript_h__
