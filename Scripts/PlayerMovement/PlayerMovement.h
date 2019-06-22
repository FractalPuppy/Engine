#ifndef  __PlayerMovement_h__
#define  __PlayerMovement_h__

class GameObject;
class JSON_value;
struct ImGuiContext;

#include "BaseScript.h"
#include "Application.h"
#include "Math/float3.h"
#include <vector>
#include <unordered_map>

#define MINIMUM_PATH_DISTANCE 400.0f

#ifdef PlayerMovement_EXPORTS
#define PlayerMovement_API __declspec(dllexport)

#else
#define PlayerMovement_API __declspec(dllimport)
#endif

#define HUB_BUTTON_Q 0
#define HUB_BUTTON_W 1
#define HUB_BUTTON_E 2
#define HUB_BUTTON_R 3
#define HUB_BUTTON_1 4
#define HUB_BUTTON_2 5
#define HUB_BUTTON_3 6
#define HUB_BUTTON_4 7

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
class DamageFeedbackUI;

#define MAX(a,b) ((a) < (b) ? (b) : (a))

enum class PlayerMovement_API SkillType
{
	NONE = 0,
	DASH,
	UPPERCUT
};

struct PlayerMovement_API PlayerSkill
{
public:
	PlayerSkill() {}
	PlayerSkill(SkillType type) : type(type) {}
	void Expose(const char* title);
	void Serialize(JSON_value* json) const;
	void DeSerialize(JSON_value* json, PlayerState* playerState);
	bool IsUsable(float playerMana) const { return available && type != SkillType::NONE && (playerMana >= manaCost && cooldownTimer <= 0); }
	float Use(float minCooldown = 0.f) { cooldownTimer = MAX(cooldown,minCooldown); maxCooldown = MAX(cooldown,minCooldown); return manaCost; }
	void Update(float deltaTime) { if (cooldownTimer > 0) cooldownTimer -= deltaTime; }
	void SetCooldown(float value) { if (type != SkillType::NONE && value > cooldownTimer) { cooldownTimer = value; maxCooldown = value; } }
	float CooldownRatio() const { return cooldownTimer > 0 ? cooldownTimer / maxCooldown : 0; }

public:
	bool available = true;
	SkillType type = SkillType::NONE;
	float manaCost = 10.f;
	float cooldown = 0.f;
	PlayerState* state = nullptr;

	float cooldownTimer = 0.f;
	float maxCooldown = 0.f;
};

struct PlayerMovement_API PlayerStats
{
public:
	void Serialize(JSON_value* json) const;
	void DeSerialize(JSON_value* json);
	void Expose(const char* sectionTitle);

	PlayerStats& operator+=(const PlayerStats& other) {
		this->health += other.health;
		this->mana += other.mana;
		this->strength += other.strength;
		this->dexterity += other.dexterity;
		this->manaRegen += other.manaRegen;
		this->hpRegen += other.hpRegen;
		return *this;
	}

	PlayerStats& operator-=(const PlayerStats& other)
	{
		this->health -= other.health;
		this->mana -= other.mana;
		this->strength -= other.strength;
		this->dexterity -= other.dexterity;
		this->manaRegen -= other.manaRegen;
		this->hpRegen -= other.hpRegen;
		return *this;
	}

public:
	float health = 0.f;
	float mana = 0.f;
	int strength  = 0;
	int dexterity = 0;

	float hpRegen = 0.f;
	float manaRegen = 0.f;
};

class PlayerMovement_API PlayerMovement : public Script
{
public:
	PlayerMovement();
	virtual ~PlayerMovement();

	void Expose(ImGuiContext* context) override;

	void Start() override;
	void Update() override;
	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;

	void OnTriggerExit(GameObject* go) override;
	void Damage(float amount);

	void Equip(const PlayerStats& equipStats);
	void UnEquip(const PlayerStats& equipStats);

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

	//void ResetCooldown(unsigned int hubButtonID);
	void UseSkill(SkillType skill);

private:
	void CheckStates(PlayerState* previous, PlayerState* current);
	void CreatePlayerStates();
	void ManaManagement();

	void ActivateHudCooldownMask(bool activate, unsigned first = HUB_BUTTON_Q, unsigned last = HUB_BUTTON_4);

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

	float walkingSpeed = 300.0f;
	float dashSpeed = 10.0f;

	float health = 100.0f;
	float mana = 100.0f;
	//bool IsManaUsed = false;
	float attackDuration = 1.0f;
	float attackTimer = 0.0f;
	
	float outCombatTimer = 0.0f;
	float outCombatMaxTime = 3.0f;
	float manaRegenTimer = 0.0f;
	float manaRegenMaxTime = 5.0f;

	PlayerStats stats = { 100.0f, 100.0f, 10U, 10U, 5.0f, 5.0f };
	std::unordered_map<SkillType, PlayerSkill*> allSkills;
	SkillType activeSkills[4] = { SkillType::NONE, SkillType::NONE, SkillType::NONE, SkillType::NONE };

	float OutOfMeshCorrectionXZ = 500.f;
	float OutOfMeshCorrectionY = 300.0f;
	float maxWalkingDistance = 10000.0f;
	ComponentAnimation* anim = nullptr;
	ComponentBoxTrigger* attackBoxTrigger = nullptr;
	ComponentBoxTrigger* hpHitBoxTrigger = nullptr;
	ComponentTransform* transform = nullptr;
	PlayerState* currentState = nullptr;

	math::float3 hpHitBoxSize = math::float3::zero;

private:
	std::vector<PlayerState*> playerStates;	
	GameObject* dustParticles = nullptr;
	GameObject* dashFX = nullptr;
	GameObject* dashMesh = nullptr;

	DamageController* damageController = nullptr;
	DamageFeedbackUI* damageUIFeedback = nullptr;
	ComponentImage* lifeUIComponent = nullptr;
	ComponentImage* manaUIComponent = nullptr;

	float hubCooldown[8]	  = { 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F };
	float hubCooldownMax[8] = { 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F };
	float hubCooldownTimer[8] = { 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F };
	ComponentImage* hubCooldownMask[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	float hubGeneralAbilityCooldown = 0.5F;
	bool showAbilityCooldowns = true;
	bool showItemCooldowns = true;
};
#endif __PlayerMovement_h__
