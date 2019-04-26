#ifndef  __BasicEnemyAIScript_h__
#define  __BasicEnemyAIScript_h__

#ifdef BasicEnemyAIScript_EXPORTS
#define BasicEnemyAIScript_API __declspec(dllexport)
#else
#define BasicEnemyAIScript_API __declspec(dllimport)
#endif

#include "BaseScript.h"
#include "Geometry/AABB.h"
#include "Math/float3.h"

enum class EnemyState
{
	WAIT,
	STANDUP,
	CHASE,
	RETURN,
	ATTACK,
	COOLDOWN,
	DEAD
};

class GameObject;
class JSON_value;

class BasicEnemyAIScript_API BasicEnemyAIScript : public Script
{
public:
	void Start() override;
	void Update() override;

	void SendAnimationTrigger(EnemyState previous, EnemyState newState);
	void Expose(ImGuiContext* context) override;

	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;

private:
	void Wait();
	void StandUp();
	void Chase();
	void ReturnToStartPosition();
	void Attack();
	void Cooldown();

	void LookAtPlayer();
	void MoveTowards(float speed) const;

public:
	EnemyState enemyState = EnemyState::WAIT;

private:
	GameObject* player = nullptr;
	std::string playerName = "Player";
	std::string playerBboxName = "PlayerMesh";
	std::string myBboxName = "EnemyMesh";

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

	// BBoxes
	math::AABB* myBbox = nullptr;		
	math::AABB* playerBbox = nullptr;

	float auxTranslation = 0.0f;
	float auxTimer = 0.0f;
};

#endif __BasicEnemyAIScript_h__
