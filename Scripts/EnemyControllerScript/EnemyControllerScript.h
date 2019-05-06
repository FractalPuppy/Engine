#ifndef  __EnemyControllerScript_h__
#define  __EnemyControllerScript_h__

#include "BaseScript.h"

#include "Geometry/AABB.h"

#ifdef EnemyControllerScript_EXPORTS
#define EnemyControllerScript_API __declspec(dllexport)
#else
#define EnemyControllerScript_API __declspec(dllimport)
#endif

class ComponentRenderer;

enum class EnemyState
{
	WAIT,
	STANDUP,
	CHASE,
	RETURN,
	LAYDOWN,
	ATTACK,
	COOLDOWN,
	DEAD
};

class EnemyControllerScript_API EnemyControllerScript : public Script
{
	void Start() override;

	void Expose(ImGuiContext* context) override;

	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;

public:
	void TakeDamage(unsigned damage);

public:
	GameObject* player = nullptr;
	std::string playerName = "Player";
	std::string playerBboxName = "PlayerMesh";
	ComponentRenderer* myRender;
	std::string myBboxName = "EnemyMesh";

	// BBoxes
	math::AABB* myBbox = nullptr;
	math::AABB* playerBbox = nullptr;

	EnemyState enemyState = EnemyState::WAIT;

private:
	int health = 100;
};

#endif __EnemyControllerScript_h__
