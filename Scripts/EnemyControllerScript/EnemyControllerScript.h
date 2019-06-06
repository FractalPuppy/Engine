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

class EnemyControllerScript_API EnemyControllerScript : public Script
{
	void Start() override;
	void Update() override;

	void Expose(ImGuiContext* context) override;

	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;

public:
	void TakeDamage(unsigned damage);
	int GetHealth() const { return actualHealth; }

	inline math::float3 GetPosition() const;					// Get position of the enemy (GO with this script attached)
	inline math::Quat GetRotation() const;						// Get rotation of the enemy (GO with this script attached)
	inline math::float3 GetPlayerPosition() const;
	inline float GetDistanceTo(math::float3& position) const;	// Get distance of the enemy to position given as argument
	inline float GetDistanceTo2D(math::float3& position) const;	// Get distance of the enemy to position given as argument only taking XZ plane as reference
	inline float GetDistanceToPlayer2D() const;

	inline bool IsCollidingWithPlayer() const;

	void Move(float speed, math::float3& direction) const;
	void Move(float speed, float& refreshTime, math::float3 position, std::vector<float3>& path) const; // Move using nav mesh
	void MoveTowards(float speed) const;
	void LookAt2D(math::float3& position);

	void OnTriggerEnter(GameObject* go) override;
public:
	GameObject* player = nullptr;
	PlayerMovement* playerMovement = nullptr;
	std::string playerName = "Player";
	std::string playerBboxName = "PlayerMesh";
	std::string myBboxName = "EnemyMesh";
	ComponentAnimation* anim = nullptr;
	ComponentRenderer* myRender;

	DamageController* damageController = nullptr;
	EnemyLifeBarController* enemyLifeBar = nullptr;
	
	// BBoxes
	math::AABB* myBbox = nullptr;

	// Hitboxes
	ComponentBoxTrigger* hpBoxTrigger = nullptr;
	ComponentBoxTrigger* attackBoxTrigger = nullptr;
	ComponentBoxTrigger* playerHitBox = nullptr;

private:
	int actualHealth = 20;
	int maxHealth = 20;

	bool isDead = false;
};

#endif __EnemyControllerScript_h__