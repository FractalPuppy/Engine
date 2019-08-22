#ifndef  __EnemyControllerScript_h__
#define  __EnemyControllerScript_h__

#include "BaseScript.h"
#include "Geometry/AABB.h"
#include <vector>

#ifdef EnemyControllerScript_EXPORTS
#define EnemyControllerScript_API __declspec(dllexport)
#else
#define EnemyControllerScript_API __declspec(dllimport)
#endif

class ComponentAnimation;
class ComponentRenderer;
class ComponentBoxTrigger;
class DamageController;
class EnemyLifeBarController;
class PlayerMovement;
class EnemyLoot;
class ExperienceController;
class ResourceMaterial;


class EnemyControllerScript_API EnemyControllerScript : public Script
{
	void Start() override;
	void Awake() override;
	void Update() override;

	void Expose(ImGuiContext* context) override;

	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;

	inline virtual EnemyControllerScript* Clone() const
	{
		return new EnemyControllerScript(*this);
	}
public:
	void TakeDamage(unsigned damage);
	int GetHealth() const { return actualHealth; }

	inline math::float3 GetPosition() const;					// Get position of the enemy (GO with this script attached)
	inline math::Quat GetRotation() const;						// Get rotation of the enemy (GO with this script attached)
	inline math::float3 GetPlayerPosition() const;
	inline void SetPosition(math::float3 newPos) const;
	inline float GetDistanceTo(math::float3& position) const;	// Get distance of the enemy to position given as argument
	inline float GetDistanceTo2D(math::float3& position) const;	// Get distance of the enemy to position given as argument only taking XZ plane as reference
	inline float GetDistanceToPlayer2D() const;

	inline bool IsCollidingWithPlayer() const;

	void Move(float speed, math::float3& direction) const;		// Warning: doesn't use nav mesh
	void Move(float speed, float& refreshTime, math::float3 position, std::vector<float3>& path) const; // Move using nav mesh
	void LookAt2D(math::float3& position);

	void OnTriggerEnter(GameObject* go) override;
public:

	bool isDead = false;
	GameObject* player = nullptr;
	PlayerMovement* playerMovement = nullptr;
	std::string playerTag = "Player";
	std::string hitMaterialName = "HitMaterial";
	ComponentAnimation* anim = nullptr;
	ComponentRenderer* myRender = nullptr;

	DamageController* damageController = nullptr;
	EnemyLifeBarController* enemyLifeBar = nullptr;

	EnemyLoot* enemyLoot = nullptr;

	ExperienceController* experienceController = nullptr;

	
	// BBoxes
	math::AABB* myBbox = nullptr;

	GameObject* myMesh = nullptr;

	// Hitboxes
	ComponentBoxTrigger* hpBoxTrigger = nullptr;
	ComponentBoxTrigger* attackBoxTrigger = nullptr;
	ComponentBoxTrigger* playerHitBox = nullptr;

	ResourceMaterial* hitMaterial = nullptr;
	ResourceMaterial* defaultMaterial = nullptr;

private:
	int actualHealth = 20;
	int maxHealth = 20;
	int experience = 20;

	float hitColorDuration = 0.2f;
	float timer = 0.f;

};

extern "C" EnemyControllerScript_API Script* CreateScript();

#endif __EnemyControllerScript_h__