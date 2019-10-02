#ifndef  __BasicEnemyAIScript_h__
#define  __BasicEnemyAIScript_h__

#ifdef BasicEnemyAIScript_EXPORTS
#define BasicEnemyAIScript_API __declspec(dllexport)
#else
#define BasicEnemyAIScript_API __declspec(dllimport)
#endif

#include "BaseScript.h"
#include "Math/float3.h"
#include <vector>

class EnemyControllerScript;
class JSON_value;
class EnemyState;
class EnemyStatePatrol;
class EnemyStateChase;
class EnemyStateReturnToStart;
class EnemyStateAttack;
class EnemyStateCooldown;
class EnemyStateDeath;
class EnemyStateFlee;


class BasicEnemyAIScript_API BasicEnemyAIScript : public Script
{
public:
	void Awake() override;
	void Start() override;
	void Update() override;

	void Expose(ImGuiContext* context) override;

	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;

	inline virtual BasicEnemyAIScript* Clone() const
	{
		return new BasicEnemyAIScript(*this);
	}
private:
	void CheckStates(EnemyState* previous, EnemyState* current);

	void DrawDebug() const;

	void OnTriggerEnter(GameObject* go) override;

public:
	EnemyState* currentState = nullptr;

	EnemyStateFlee* flee = nullptr;
	EnemyStatePatrol* patrol = nullptr;
	EnemyStateChase* chase = nullptr;
	EnemyStateReturnToStart* returnToStart = nullptr;
	EnemyStateAttack* attack = nullptr;
	EnemyStateCooldown* cooldown = nullptr;
	EnemyStateDeath* death = nullptr;

	bool drawDebug = true;				// If true will draw all debug for enemy behaviour

	// Patrol variables
	float activationDistance = 1000.0f;	// Distance to player needed to start chasing the player (only X,Z axis is taken into account)
	float attackRange = 400.f;

	// Chase variables
	float chaseSpeed = 2.0f;			// Tranlation speed when chasing player

	// Return variables
	math::float3 startPosition;
	float returnDistance = 150.f;		// Distance to player to stop chasing player and return to start position
	float returnSpeed = 1.0f;			// Tranlation speed towards start position

	// Attack variables
	float attackDuration = 1.0f;
	float attackDamage = 20.0f;

	// Cooldown variables
	float cooldownTime = 1.0f;			// Seconds to wait between attacks

	bool scared = false;

	EnemyControllerScript* enemyController = nullptr;

protected:
	std::vector<EnemyState*> enemyStates;
};

extern "C" BasicEnemyAIScript_API Script* CreateScript();

#endif __BasicEnemyAIScript_h__
