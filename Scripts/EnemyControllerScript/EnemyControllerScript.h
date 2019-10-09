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
class ExperienceController;
class ResourceMaterial;
class CombatAudioEvents;
class LootDropScript;
class WorldControllerScript;

enum class EnemyType {SKELETON, MINER, SORCERER, SPINNER, BANDOLERO};

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
	void TakeDamage(unsigned damage, int type = 1);
	int GetHealth() const { return actualHealth; }
	int GetMaxHealth() const { return maxHealth; }
	int IsDeadCritOrSkill() const { return isDeadByCritOrSkill; } //0 normal - 1 crit or skill

	inline math::float3 GetPosition() const;					// Get position of the enemy (GO with this script attached)
	inline math::Quat GetRotation() const;						// Get rotation of the enemy (GO with this script attached)
	inline math::float3 GetPlayerPosition() const;
	inline void SetPosition(math::float3 newPos) const;
	inline float GetDistanceTo(math::float3& position) const;	// Get distance of the enemy to position given as argument
	inline float GetDistanceTo2D(math::float3& position) const;	// Get distance of the enemy to position given as argument only taking XZ plane as reference
	inline float GetDistanceToPlayer2D() const;

	inline ComponentRenderer* GetMainRenderer() const;			// Gets the main renderer of the enemy (first ComponentRenderer stored in myRenders)

	inline bool IsCollidingWithPlayer() const;

	void Move(float speed, math::float3& direction) const;		// Warning: doesn't use nav mesh
	bool Move(float speed, float& refreshTime, math::float3 position, std::vector<float3>& path) const; // Move using nav mesh
	bool IsIdle() const;
	bool IsStuck() const;
	void Stop();
	void LookAt2D(const math::float3& position);

	void OnTriggerEnter(GameObject* go) override;

public:

	bool isDead = false;
	bool hasFreeRotation = false;
	GameObject* player = nullptr;
	PlayerMovement* playerMovement = nullptr;
	std::string playerTag = "Player";
	std::string enemyCursor = "RedGlow.cur";
	std::string hitMaterialName = "HitMaterial";
	ComponentAnimation* anim = nullptr;
	std::vector<ComponentRenderer*> myRenders;

	DamageController* damageController = nullptr;
	EnemyLifeBarController* enemyLifeBar = nullptr;

	ExperienceController* experienceController = nullptr;

	
	// BBoxes
	math::AABB* myBbox = nullptr;

	GameObject* myMesh = nullptr;

	// Hitboxes
	ComponentBoxTrigger* hpBoxTrigger = nullptr;
	ComponentBoxTrigger* attackBoxTrigger = nullptr;
	ComponentBoxTrigger* playerHitBox = nullptr;

	ResourceMaterial* hitMaterial = nullptr;				// Material applied to all enemy meshes on hit
	std::vector<ResourceMaterial*> defaultMaterials;		// Vector containing default materials of the enemy meshes

	CombatAudioEvents* combataudioevents = nullptr;

	// Enemy Type and level (1 = NORMAL, 2 = HARD, 3 = BOSS)
	int enemyLevel = 1u;			
	EnemyType enemyType = EnemyType::SKELETON;

private:
	int actualHealth = 20;
	int maxHealth = 20;
	int experience = 20;
	mutable float currentSpeed = 100.f;
	int isDeadByCritOrSkill = 0;

	float hitColorDuration = 0.2f;
	float hitColorTimer = 0.f;
	bool enemyHit = false;

	// Loot variables
	LootDropScript* lootDrop = nullptr;		// If != nullptr on enemy death will drop item(s) (The variable is set automatically if the LootDropScript is found on Start)
	WorldControllerScript* currentWorldControllerScript = nullptr;
	bool lootDropped = false;
	float deathTimer = 0.0f;				
	float lootDelay = 1.0f;					// Time since enemy died untill loot is spawned
	float lootRadius = 100.0f;				// Distance from enemy position to drop Items around (only if Items to drop > 1)
};

extern "C" EnemyControllerScript_API Script* CreateScript();

#endif __EnemyControllerScript_h__