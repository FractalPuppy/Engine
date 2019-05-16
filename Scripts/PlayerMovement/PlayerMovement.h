#ifndef  __PlayerMovement_h__
#define  __PlayerMovement_h__

class GameObject;
class JSON_value;
struct ImGuiContext;

#include "BaseScript.h"
#include "Application.h"
#include "Math/float3.h"
#include <vector>

#ifdef PlayerMovement_EXPORTS
#define PlayerMovement_API __declspec(dllexport)

#else
#define PlayerMovement_API __declspec(dllimport)
#endif

class ComponentAnimation;
class JSON_value;
struct ImGuiContext;
class PlayerState;
class PlayerStateFirstAttack;
class PlayerStateSecondAttack;
class PlayerStateThirdAttack;
class PlayerStateIdle;
class PlayerStateDash;
class PlayerStateDeath;
class PlayerStateUppercut;
class PlayerStateWalk;

class PlayerMovement_API PlayerMovement : public Script
{
public:
	void Expose(ImGuiContext* context) override;

	void Start() override;
	void Update() override;
	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;

	//Abstract input
	bool IsAtacking();
	bool IsMoving();
	bool IsUsingFirstSkill();
	bool IsUsingSecondSkill();
	bool IsUsingThirdSkill();
	bool IsUsingFourthSkill();
	bool IsUsingFirstItem();
	bool IsUsingSecondItem();
	bool IsUsingThirdItem();
	bool IsUsingFourthItem();

	void CheckStates(PlayerState* previous, PlayerState* current);

	void Damage(float amount);
public:
	bool isPlayerDead = false;
	float3 currentPosition = float3(0, 0, 0); //TODO ZERO

	PlayerStateFirstAttack* firstAttack = nullptr;
	PlayerStateSecondAttack* secondAttack = nullptr;
	PlayerStateThirdAttack* thirdAttack = nullptr;
	PlayerStateIdle* idle = nullptr;
	PlayerStateDash* dash = nullptr;
	PlayerStateDeath* death = nullptr;
	PlayerStateUppercut* uppercut = nullptr;
	PlayerStateWalk* walk = nullptr;

	std::vector<float3>path;
	unsigned pathIndex = 0;
public:

	float speed = 2.0f;
	float dashSpeed = 10.0f;
	float health = 100.0f;
	float attackDuration = 1.0f;
	float attackTimer = 0.0f;
	ComponentAnimation* anim = nullptr;
	PlayerState* currentState = nullptr;
public:

	float dashDuration = 1.f;
	float firstAttackDuration = 1.f;
	float secondAttackDuration = 1.f;
	float thirdAttackDuration = 1.f;
	float uppercutDuration = 1.f;

	Application* Appl = nullptr;
private:
	std::vector<PlayerState*> playerStates;	
};
#endif __PlayerMovement_h__
