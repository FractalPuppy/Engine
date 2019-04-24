#ifndef  __BasicEnemyAIScript_h__
#define  __BasicEnemyAIScript_h__

#include "BaseScript.h"

#ifdef BasicEnemyAIScript_EXPORTS
#define BasicEnemyAIScript_API __declspec(dllexport)
#else
#define BasicEnemyAIScript_API __declspec(dllimport)
#endif

class GameObject;

enum class EnemyState
{
	STANDUP,
	CHASE,
	ATTACK,
	COOLDOWN,
	DEAD
};

class BasicEnemyAIScript_API BasicEnemyAIScript : public Script
{
public:
	void Start() override;
	void Update() override;

	void Expose(ImGuiContext* context) override;

private:
	void StandUp();
	void Chase();

	GameObject* GetGameObjectRecursiveByName(GameObject* gameObject, const char* name);

public:
	EnemyState enemyState = EnemyState::STANDUP;

private:
	GameObject* player = nullptr;
	std::string playerName = "Player";

	// Stand-Up variables
	float standupSpeed = 1.0f;
	float yTranslation = 20.0f;

	float auxTranslation = 0.0f;

	math::AABB* myBbox = nullptr;
};

#endif __BasicEnemyAIScript_h__
