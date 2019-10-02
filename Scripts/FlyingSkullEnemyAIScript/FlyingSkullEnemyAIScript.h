#ifndef  __FlyingSkullEnemyAIScript_h__
#define  __FlyingSkullEnemyAIScript_h__

#ifdef FlyingSkullEnemyAIScript_EXPORTS
#define FlyingSkullEnemyAIScript_API __declspec(dllexport)
#else
#define FlyingSkullEnemyAIScript_API __declspec(dllimport)
#endif

#include "BaseScript.h"
#include "math/float3.h"
#include <vector>

class GameObject;
class EnemyControllerScript;
class ProjectileScript;
class JSON_value;
class EnemyState;
class EnemyStatePatrol;
class EnemyStateChase;
class EnemyStateReturn;
class EnemyStateAttack;
class EnemyStateCooldown;
class EnemyStateDeath;
class EnemyStateFlee;


class FlyingSkullEnemyAIScript_API FlyingSkullEnemyAIScript : public Script
{
	void Awake() override;
	void Start() override;
	void Update() override;

	void Expose(ImGuiContext* context) override;

	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;

	inline virtual FlyingSkullEnemyAIScript* Clone() const
	{
		return new FlyingSkullEnemyAIScript(*this);
	}

public:

	//IA parameters
	float cooldownDuration = 1.0f;
	float fleeDuration = 1.5f;
	float attackDuration = 1.0f;
	float activationDistance = 1000.0f;
	float disengageDistance = 1500.0f;
	float maxAttackRange = 700.0f;
	float minAttackRange = 500.0f;
	float runSpeed = 150.0f;
	float offsetHeight = 120.0f;

	//animation paramteres

	//Skull
	float movementConstant = 20.0f;
	float idleMovementSpeed = 5.0f;
	float modifiedSpeed = 0.0f;
	float maxIdleSpeed = 20.0f;
	float minIdleSpeed = 2.0f;
	float acceleration = 0.0f;
	float angle = 0.0f;

	//orbit bones
	float firstRotationSpeed = 0.02f;
	float secondRotationSpeed = 0.03f;

	float projectileDelay = 0.5f;

	GameObject* projectileGO = nullptr;
	ProjectileScript* projectileScript = nullptr;

public:
	//hierarchy

	GameObject* boneCenter = nullptr;
	GameObject* firstSpin = nullptr;
	GameObject* secondSpin = nullptr;
	GameObject* firstRotation = nullptr;
	GameObject* secondRotation = nullptr;
	GameObject* firstBone = nullptr;
	GameObject* secondBone = nullptr;
	GameObject* particles = nullptr;

	void Movement();
	void OrbitMovement();

public:

	EnemyStateAttack* attack = nullptr;
	EnemyStateChase* chase = nullptr;
	EnemyStateFlee* flee = nullptr;
	EnemyStateCooldown* cooldown = nullptr;
	EnemyStateReturn* returnToStart = nullptr;
	EnemyStateDeath* death = nullptr;
	EnemyStatePatrol* patrol = nullptr;

	EnemyState* currentState = nullptr;

	EnemyControllerScript* enemyController = nullptr;

	math::float3 startPosition = math::float3::zero;
private:
	std::vector<EnemyState*> enemyStates;

	void CheckStates(EnemyState* previous);
};

#endif __FlyingSkullEnemyAIScript_h__
