#ifndef  __BasicEnemyAIScript_h__
#define  __BasicEnemyAIScript_h__

#ifdef BasicEnemyAIScript_EXPORTS
#define BasicEnemyAIScript_API __declspec(dllexport)
#else
#define BasicEnemyAIScript_API __declspec(dllimport)
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

class BasicEnemyAIScript_API BasicEnemyAIScript : public Script
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

#endif __BasicEnemyAIScript_h__
