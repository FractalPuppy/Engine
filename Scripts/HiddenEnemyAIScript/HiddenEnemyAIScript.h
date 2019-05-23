#ifndef  __HiddenEnemyAIScript_h__
#define  __HiddenEnemyAIScript_h__

#ifdef HiddenEnemyAIScript_EXPORTS
#define HiddenEnemyAIScript_API __declspec(dllexport)
#else
#define HiddenEnemyAIScript_API __declspec(dllimport)
#endif

#include "BaseScript.h"
#include "Math/float3.h"

class GameObject;
class ComponentAnimation;
class ComponentRenderer;
class PlayerMovement;
class EnemyControllerScript;
class JSON_value;
enum class EnemyState;

enum class EnemyState
{
	WAIT,
	SHOW_UP,
	CHASE,
	RETURN,
	HIDE,
	ATTACK,
	COOLDOWN,
	DEAD
};

class HiddenEnemyAIScript_API HiddenEnemyAIScript : public Script
{
public:
	void Start() override;
	void Update() override;

	void Expose(ImGuiContext* context) override;

	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;

private:
	void Wait();
	void StandUp();
	void Chase();
	void ReturnToStartPosition();
	void Laydown();
	void Attack();
	void Cooldown();
	void Die();

	void MoveTowards(float speed) const;
	void CheckStateChange(EnemyState previous, EnemyState newState);

private:

	EnemyState enemyState = EnemyState::WAIT;

	// Wait variables
	float activationDistance = 100.0f;	// Distance to player needed to start chasing the player (only X,Z axis is taken into account)

	// Stand-Up variables
	float standupSpeed = 1.0f;			// Tranlation speed on stand-up
	float yTranslation = 20.0f;			// Y axis translation on stand-up 

	// Chase variables
	float chaseSpeed = 2.0f;			// Tranlation speed when chasing player

	// Return variables
	math::float3 startPosition;
	float returnDistance = 150.f;		// Distance to player to stop chasing player and return to start position
	float returnSpeed = 1.0f;			// Tranlation speed towards start position

	// Cooldown variables
	float cooldownTime = 1.0f;			// Seconds to wait between attacks

	float auxTranslation = 0.0f;
	float auxTimer = 0.0f;

	//Damage variables
	float damage = 20.0f;

	ComponentAnimation* anim = nullptr;

	EnemyControllerScript* enemyController;
	PlayerMovement* playerScript;
};

#endif __HiddenEnemyAIScript_h__


