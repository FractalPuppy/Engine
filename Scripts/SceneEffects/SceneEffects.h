#ifndef  __SceneEffects_h__
#define  __SceneEffects_h__

#include "BaseScript.h"

#ifdef SceneEffects_EXPORTS
#define SceneEffects_API __declspec(dllexport)
#else
#define SceneEffects_API __declspec(dllimport)
#endif

#include <vector>
#include "Math/float3.h"

#define BAT_SPEED 450.f
#define BAT_STOP_DISTANCE 100.f

#define SPIDER_SPEED .3f
#define SPIDER_STOP_DISTANCE .05f

class ComponentLight;

class SceneEffects_API SceneEffects : public Script
{

	enum class Behaviours
	{
		NONE,
		BATS,
		STORM,
		SPIDER
	};

	void Expose(ImGuiContext* context) override;
	void Start() override;
	void Update() override;


	inline virtual SceneEffects* Clone() const
	{
		return new SceneEffects(*this);
	}

	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;

	const char* behaviours[4]  = {"None", "Bats", "Storm", "Spider" };
	const char* item_current = behaviours[0];

	Behaviours behaviour = Behaviours::NONE;
	int currentBehaviour = 0;

	//Bats stuff

	struct Bat
	{
		math::float3 destination = math::float3::zero;
		GameObject* gameobject = nullptr;
	};

	std::vector<Bat> bats;
	std::vector<math::float3> waypoints;

	int batAmount = 5;
	
	//Storm stuff
	
	ComponentLight* originalDirectionalLight = nullptr;
	std::vector<ComponentLight*> lightingLights;

	float stormTimer = 0.f;
	float stormDuration = 1.f;
	float stormCooldown = 4.f;
	float stormCooldownTimer = 0.f;

	//Spider stuff

	GameObject* spider = nullptr;
	std::vector<math::float3> spiderWaypoints;
	math::float3 spiderDestination = math::float3::zero;
	float spiderTimer = .0f;
};


extern "C" SceneEffects_API Script* CreateScript();

#endif __SceneEffects_h__
