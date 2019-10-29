#ifndef  __RangeEnemyAIScript_h__
#define  __RangeEnemyAIScript_h__

#ifdef RangeEnemyAIScript_EXPORTS
#define RangeEnemyAIScript_API __declspec(dllexport)
#else
#define RangeEnemyAIScript_API __declspec(dllimport)
#endif

#include "BaseScript.h"
#include "Math/float3.h"
#include <vector>

class GameObject;
class ComponentBoxTrigger;
class EnemyControllerScript;
class ProjectileScript;
class JSON_value;
class EnemyState;
class EnemyStatePatrol;
class EnemyStateChase;
class EnemyStateFlee;
class EnemyStateReturnToStart;
class EnemyStateAttack;
class EnemyStateCooldown;
class EnemyStateDeath;

class RangeEnemyAIScript_API RangeEnemyAIScript : public Script
{
public:
	void Start() override;
	void Update() override;

	void Expose(ImGuiContext* context) override;

	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;

	inline virtual RangeEnemyAIScript* Clone() const
	{
		return new RangeEnemyAIScript(*this);
	}
private:
	void ProjectileImpactFX();
	void CheckStates(EnemyState* previous, EnemyState* current);
	GameObject* InstantiateProjectile(const char* projectilePrefab);
	void DrawDebug() const;

	void OnTriggerEnter(GameObject* go);

public:
	EnemyState* currentState = nullptr;

	EnemyStatePatrol* patrol = nullptr;
	EnemyStateChase* chase = nullptr;
	EnemyStateFlee* flee = nullptr;
	EnemyStateReturnToStart* returnToStart = nullptr;
	EnemyStateAttack* attack = nullptr;
	EnemyStateCooldown* cooldown = nullptr;
	EnemyStateDeath* death = nullptr;

	bool drawDebug = true;				// If true will draw all debug for enemy behaviour

	// Patrol variables
	float activationDistance = 100.0f;	// Distance to player needed to start chasing the player (only X,Z axis is taken into account)

	// Get in position variables
	float runSpeed = 2.0f;				// Tranlation speed when getting in position to attack player
	float maxAttackDistance = 80.0f;
	float minAttackDistance = 20.0f;

	// Return variables
	math::float3 startPosition;
	float disengageDistance = 150.f;		// Distance to player to stop chasing player and return to start position
	float returnSpeed = 1.0f;				// Tranlation speed towards start position

	// Attack variables
	float attackDuration = 1.0f;
	float attackDamage = 20.0f;

	int numberOfProjectiles = 1u;

	GameObject* projectile1 = nullptr;
	GameObject* projectile2 = nullptr;
	GameObject* projectile3 = nullptr;
	GameObject* projectileExplosion = nullptr;

	float projectileDelay1 = 0.f;
	float projectileDelay2 = 0.6f;
	float projectileDelay3 = 1.0f;
	float explosionDuration = 0.15f;
	float explosionTimer = 0.0f;

	// Cooldown variables
	float cooldownTime = 1.0f;			// Seconds to wait between attacks

	ComponentBoxTrigger* boxTrigger = nullptr;
	EnemyControllerScript* enemyController = nullptr;
	ProjectileScript* projectileScript1 = nullptr;
	ProjectileScript* projectileScript2 = nullptr;
	ProjectileScript* projectileScript3 = nullptr;

private:
	std::vector<EnemyState*> enemyStates;
};

extern "C" RangeEnemyAIScript_API Script* CreateScript();

#endif __RangeEnemyAIScript_h__
