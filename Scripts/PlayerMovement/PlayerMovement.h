#ifndef  __PlayerMovement_h__
#define  __PlayerMovement_h__

class GameObject;
class JSON_value;
struct ImGuiContext;

#include "BaseScript.h"
#include "Application.h"
#include "Math/float3.h"
#include <vector>

#define MINIMUM_PATH_DISTANCE 400.0f

#ifdef PlayerMovement_EXPORTS
#define PlayerMovement_API __declspec(dllexport)

#else
#define PlayerMovement_API __declspec(dllimport)
#endif

class ComponentAnimation;
class ComponentTransform;
class ComponentBoxTrigger;
class ComponentImage;
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
class DamageController;

class PlayerMovement_API PlayerMovement : public Script
{
public:
	void Expose(ImGuiContext* context) override;


	void Start() override;
	void Update() override;
	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;

	void OnTriggerExit(GameObject* go) override;
	void Damage(float amount);

	//Abstract input
	bool IsAtacking() const;
	bool IsMoving() const;
	bool IsUsingFirstSkill() const;
	bool IsUsingSecondSkill() const;
	bool IsUsingThirdSkill() const;
	bool IsUsingFourthSkill() const;
	bool IsUsingFirstItem() const;
	bool IsUsingSecondItem() const;
	bool IsUsingThirdItem() const;
	bool IsUsingFourthItem() const;

private:
	void CheckStates(PlayerState* previous, PlayerState* current);
	void CreatePlayerStates();
	void ManaManagement();

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

	float walkingSpeed = 100.0f;
	float dashSpeed = 10.0f;
	const float fullHealth = 100.0f;
	float health = fullHealth;
	const float fullMana = 100.0f;
	float mana = fullMana;
	bool IsManaUsed = false;
	float attackDuration = 1.0f;
	float attackTimer = 0.0f;
	ComponentAnimation* anim = nullptr;
	ComponentBoxTrigger* attackBoxTrigger = nullptr;
	ComponentBoxTrigger* hpHitBoxTrigger = nullptr;
	ComponentTransform* transform = nullptr;
	PlayerState* currentState = nullptr;

	math::float3 hpHitBoxSize = math::float3::zero;

	bool canInteract = true;

private:
	std::vector<PlayerState*> playerStates;	
	GameObject* dustParticles = nullptr;
	GameObject* dashFX = nullptr;
	GameObject* dashMesh = nullptr;

	DamageController* damageController = nullptr;
	ComponentImage* lifeUIComponent = nullptr;
	ComponentImage* manaUIComponent = nullptr;
};
#endif __PlayerMovement_h__
