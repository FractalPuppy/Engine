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

#define HUD_BUTTON_RC 0
#define HUD_BUTTON_1 1
#define HUD_BUTTON_2 2
#define HUD_BUTTON_3 3
#define HUD_BUTTON_4 4
#define HUD_BUTTON_Q 5
#define HUD_BUTTON_W 6
#define HUD_BUTTON_E 7
#define HUD_BUTTON_R 8

#define NUMBER_OF_PLAYERSTATES 7

#define MAX_BOMB_DROP_SCALE 200.f
#define MAX_BOMB_DROP_WAVE_SCALE 240.f
#define BOMB_DROP_ROT 2.5f

#define MACHETE_RAIN_START_HEIGHT 3300.0f
#define MACHETE_RAIN_SPEED 4000.0f
#define MACHETE_RAIN_HORIZONTAL_SPEED 40.0f
#define MACHETE_AMOUNT 40u
#define MACHETE_SKILL_RANGE 900.f

class ComponentAnimation;
class ComponentTransform;
class ComponentBoxTrigger;
class ComponentImage;
class ComponentRenderer;
class JSON_value;
struct ImGuiContext;
class PlayerState;
class PlayerStateAttack;
class PlayerStateIdle;
class PlayerStateDeath;
class PlayerStateWalk;
class PlayerStateWalkToHitEnemy;
class PlayerStateWalkToPickItem;
class DamageController;
class DamageFeedbackUI;
class ComponentAudioSource;
class ComponentCamera;
class ItemPicker;

class Text;
class BasicSkill;
class SliceSkill;
class ChainAttackSkill;
class DashSkill;
class BombDropSkill;
class CircularAttackSkill;
class StompSkill;
class RainSkill;

#define MAX(a,b) ((a) < (b) ? (b) : (a))

enum class PlayerMovement_API SkillType
{
	DASH,
	CIRCULAR,
	BOMB_DROP,
	RAIN,
	SLICE,
	STOMP,
	CHAIN = 10,
	NONE = 20
};

struct PlayerMovement_API PlayerSkill
{
public:
	PlayerSkill() {}
	PlayerSkill(SkillType type, float manaCost = 10.0f, float cooldown = 0.0f) : type(type), manaCost(manaCost), cooldown(cooldown){}
	void Expose(const char* title);
	void Serialize(JSON_value* json) const;
	void DeSerialize(JSON_value* json, BasicSkill* playerSkill);

	bool IsUsable(float playerMana) const { return available && type != SkillType::NONE && (playerMana >= manaCost && cooldownTimer <= 0); }
	float Use(float minCooldown = 0.f) { cooldownTimer = MAX(cooldown, minCooldown); maxCooldown = MAX(cooldown, minCooldown); return manaCost; }
	void Update(float deltaTime) { if (cooldownTimer > 0) cooldownTimer -= deltaTime; }
	void SetCooldown(float value) { if (type != SkillType::NONE && value > cooldownTimer) { cooldownTimer = value; maxCooldown = value; } }
	float CooldownRatio() const { return cooldownTimer > 0 ? cooldownTimer / maxCooldown : 0; }

public:
	bool available = true;
	SkillType type = SkillType::NONE;
	float manaCost = 10.f;
	float cooldown = 0.f;
	BasicSkill* skill = nullptr;

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

	inline virtual PlayerMovement* Clone() const
	{
		return new PlayerMovement(*this);
	}

	void OnTriggerExit(GameObject* go) override;
	void Damage(float amount);

	void Equip(const PlayerStats& equipStats);
	void Equip(const PlayerStats& equipStats, unsigned itemType, unsigned meshUID, unsigned materialUID);	// Equip item stats and mesh (Calls EquipMesh())
	void EquipMesh(unsigned itemType, unsigned meshUID, unsigned materialUID);								// Equip only the item mesh
	void UnEquip(const PlayerStats& equipStats, unsigned itemType);
	void ConsumeItem(const PlayerStats& equipStats);

	void OnAnimationEvent(std::string name) override;

	//Abstract input. TODO: Now only returns true for skills, adapt for items
	inline bool IsMovingToItem() const { return itemClicked != nullptr;};
	bool IsAttacking() const;
	bool IsMoving() const;
	bool IsMovingToAttack() const;
	bool IsPressingMouse1() const;
	bool IsUsingRightClick() const;
	bool IsUsingOne() const;
	bool IsUsingTwo() const;
	bool IsUsingThree() const;
	bool IsUsingFour() const;
	bool IsUsingQ() const;
	bool IsUsingW() const;
	bool IsUsingE() const;
	bool IsUsingR() const;
	bool IsUsingSkill() const;
	bool IsExecutingSkill() const;

	void PrepareSkills() const;

	void CheckSkillsInput();
	void ResetCooldown(unsigned int hubButtonID);
	void UseSkill(SkillType skill);
	void AssignSkill(SkillType skill, int position);

	void ToggleMaxStats();
	void ToggleInfiniteHealth();
	void ToggleInfiniteMana();
	void SavePlayerStats();

private:
	void CheckStates(PlayerState* previous, PlayerState* current);
	void CreatePlayerStates();
	void ManaManagement();

	void ActivateHudCooldownMask(bool activate, unsigned first = HUD_BUTTON_Q, unsigned last = HUD_BUTTON_4);

	float DistPlayerToMouse() const;

	// Skills
	void CreatePlayerSkills();

	void UpdateUIStats();
	void InitializeUIStatsObjects();
public:
	bool isPlayerDead = false;
	float3 currentPosition = float3(0, 0, 0); //TODO ZERO

	PlayerStateAttack* attack = nullptr;
	PlayerStateIdle* idle = nullptr;
	PlayerStateDeath* death = nullptr;
	PlayerStateWalk* walk = nullptr;
	PlayerStateWalkToHitEnemy* walkToHit = nullptr;
	PlayerStateWalkToPickItem* walkToPickItem = nullptr;
	//walking to hit player orientation necessary info:
	bool enemyTargeted = false;
	GameObject* enemyTarget = nullptr;

	//item picked
	ItemPicker* itemClicked = nullptr;
	bool stoppedGoingToItem = false;

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

	PlayerStats stats = { 150.0f, 100.0f, 10U, 10U, 5.0f, 5.0f };
	PlayerStats previousStats;

	float OutOfMeshCorrectionXZ = 500.f;
	float OutOfMeshCorrectionY = 300.0f;
	float maxWalkingDistance = 50000.0f;
	float straightPathingDistance = 2000.0f;
	ComponentAnimation* anim = nullptr;
	ComponentBoxTrigger* attackBoxTrigger = nullptr;
	ComponentBoxTrigger* hpHitBoxTrigger = nullptr;
	ComponentTransform* transform = nullptr;
	PlayerState* currentState = nullptr;

	GameObject* playerCamera = nullptr;

	math::float3 hpHitBoxSize = math::float3::zero;

	bool canInteract = true;

	// Probability of critical attack
	float criticalChance = 10.0f;

	// Skills
	BasicSkill* currentSkill = nullptr;
	ChainAttackSkill* chain = nullptr;
	DashSkill* dash = nullptr;
	SliceSkill* slice = nullptr;
	BombDropSkill* bombDrop = nullptr;
	CircularAttackSkill* circular = nullptr;
	StompSkill* stomp = nullptr;
	RainSkill* rain = nullptr;
	float basicAttackRange = 200.f;

	bool macheteRainActivated = false;
	bool shaking = false;

	std::unordered_map<SkillType, PlayerSkill*> allSkills;

	// Vector with all skill slots (Right-Click, 1, 2, 3, 4, Q, W, E, R)
	SkillType assignedSkills[9] = { SkillType::NONE, SkillType::NONE, SkillType::NONE, SkillType::NONE, SkillType::NONE, SkillType::NONE, SkillType::NONE, SkillType::NONE, SkillType::NONE };

	//Audio
	ComponentAudioSource* gotHitAudio = nullptr;
	GameObject* slashTrail = nullptr;


	float deltatime;

private:
	std::vector<PlayerState*> playerStates;	
	GameObject* dustParticles = nullptr;
	GameObject* dashFX = nullptr;
	GameObject* bombDropParticles = nullptr;
	GameObject* bombDropParticlesLanding = nullptr;
	GameObject* bombDropMesh1 = nullptr;
	GameObject* bombDropMesh2 = nullptr;
	math::float3 bombDropMesh1Scale = math::float3::one;
	math::float3 bombDropMesh2Scale = math::float3::one;
	bool bombDropExpanding = false;
	float bombDropGrowRate = 1.3f;
	float bombDropWaveGrowRate = 1.05f;

	DamageController* damageController = nullptr;
	DamageFeedbackUI* damageUIFeedback = nullptr;
	ComponentImage* lifeUIComponent = nullptr;
	ComponentImage* manaUIComponent = nullptr;
	Text* uiHealthText = nullptr;
	Text* uiDexterityText = nullptr;
	Text* uiStrengthText = nullptr;
	Text* uiManaText = nullptr;

	float closestDistToPlayer = 31000.0f;
	float hubCooldown[9]	  = { 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F };
	float hubCooldownMax[9] = { 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F };
	float hubCooldownTimer[9] = { 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F };
	ComponentImage* hubCooldownMask[9] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	float hubGeneralAbilityCooldown = 0.5F;
	bool showAbilityCooldowns = true;
	bool showItemCooldowns = true;

	bool hasMaxStats = false;
	bool hasInfiniteHealth = false;
	bool hasInfiniteMana = false;

	// Player equippable parts (Weapon, Helmet)
	ComponentRenderer* weaponRenderer = nullptr;
	ComponentRenderer* helmetRenderer = nullptr;
};

extern "C" PlayerMovement_API Script* CreateScript();

#endif __PlayerMovement_h__
