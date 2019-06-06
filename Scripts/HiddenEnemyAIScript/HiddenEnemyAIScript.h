#ifndef  __HiddenEnemyAIScript_h__
#define  __HiddenEnemyAIScript_h__

#ifdef HiddenEnemyAIScript_EXPORTS
#define HiddenEnemyAIScript_API __declspec(dllexport)
#else
#define HiddenEnemyAIScript_API __declspec(dllimport)
#endif

#include "BaseScript.h"
#include "Math/float3.h"
#include <vector>

class GameObject;
class EnemyControllerScript;
class JSON_value;
class EnemyState;
class EnemyStateWait;
class EnemyStateShowUp;
class EnemyStateChase;
class EnemyStateReturnToStart;
class EnemyStateHide;
class EnemyStateAttack;
class EnemyStateCooldown;
class EnemyStateDeath;

class HiddenEnemyAIScript_API HiddenEnemyAIScript : public Script
{
public:
	void Start() override;
	void Update() override;

	void Expose(ImGuiContext* context) override;

	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;

private:
	void CheckStates(EnemyState* previous, EnemyState* current);
	void DrawDebug() const;

public:
	EnemyState* currentState = nullptr;

	EnemyStateWait* wait = nullptr;
	EnemyStateShowUp* showUp = nullptr;
	EnemyStateChase* chase = nullptr;
	EnemyStateReturnToStart* returnToStart = nullptr;
	EnemyStateHide* hide = nullptr;
	EnemyStateAttack* attack = nullptr;
	EnemyStateCooldown* cooldown = nullptr;
	EnemyStateDeath* death = nullptr;

	bool drawDebug = true;				// If true will draw all debug for enemy behaviour

	// Wait variables
	float activationDistance = 100.0f;	// Distance to player needed to start chasing the player (only X,Z axis is taken into account)

	// Show-Up variables
	float showUpSpeed = 1.0f;			// Tranlation speed on stand-up
	float yTranslation = 20.0f;			// Y axis translation on stand-up 

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

	EnemyControllerScript* enemyController = nullptr;

private:
	std::vector<EnemyState*> enemyStates;
};

#endif __HiddenEnemyAIScript_h__


