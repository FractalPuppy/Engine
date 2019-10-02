#ifndef  __BossBehaviourScript_h__
#define  __BossBehaviourScript_h__

#include "BaseScript.h"
#include <vector>
#include "Math/float3.h"
#include "Math/Quat.h"

#ifdef BossBehaviourScript_EXPORTS
#define BossBehaviourScript_API __declspec(dllexport)
#else
#define BossBehaviourScript_API __declspec(dllimport)
#endif

enum class BossPhase
{
	None,
	First,
	Second,
	Third
};

enum class BossSkill
{
	None,
	Teleport,
	Aoe,
	Summon,
	Bombs
};

enum class TPlocations
{
	None,
	Top,
	Bottom,
	Left,
	Right
};

enum class TPState
{
	None,
	FadeOut,
	Relocate,
	FadeIn,
	Projectiles,
	Finished
};

class BossState;
class BossStateCast;
class BossStatePreCast;
class BossStateIdle;
class BossStateDeath;
class BossStateNotActive;
class BossStateActivated;
class BossStateInterPhase;
class BossStateSummonArmy;
class BossStateCutScene;
class EnemyControllerScript;
class PlayerMovement;
class GameLoop;

class BossBehaviourScript_API BossBehaviourScript : public Script
{
public:
	void Awake() override;
	void Start() override;
	void Update() override;

	void Expose(ImGuiContext* context) override;

	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;

	inline virtual BossBehaviourScript* Clone() const
	{
		return new BossBehaviourScript(*this);
	}
public:
	TPlocations currentLocation = TPlocations::None;
	BossPhase bossPhase = BossPhase::None;
	BossState* currentState = nullptr;
	BossSkill lastUsedSkill = BossSkill::None;
	BossSkill secondLastUsedSkill = BossSkill::None;

	EnemyControllerScript* enemyController = nullptr;

	BossStateActivated* activated = nullptr;
	BossStateNotActive* notActive = nullptr;
	BossStateIdle* idle = nullptr;
	BossStatePreCast* precast = nullptr;
	BossStateSummonArmy* summonArmy = nullptr;
	BossStateInterPhase* interPhase = nullptr;
	BossStateDeath* death = nullptr;
	BossStateCast* cast = nullptr;
	BossStateCutScene* cutscene = nullptr;

public:
	GameObject* gameLoopGO = nullptr;
	GameLoop* gLoop = nullptr;

	GameObject* playerGO = nullptr;
	PlayerMovement* playerScript = nullptr;

	GameObject* firstMeshFloor = nullptr;
	GameObject* secondMeshFloor = nullptr;
	GameObject* mainFirstFloor = nullptr;
	GameObject* mainSecondFloor = nullptr;
	GameObject* mainThirdFloor = nullptr;
	GameObject* mainFourthFloor = nullptr;
	GameObject* mainFifthFloor = nullptr;
	GameObject* mainSixthFloor = nullptr;

	std::vector<GameObject*> floorBossGOs;
	std::vector<GameObject*> floorMainGOs;

	void PrepareBossFight(std::vector<GameObject*>& vectorGOs);
	void GenerateNewNavigability(std::vector<GameObject*>& vectorGOs);

public:
	float firstHealthThreshold = 0.75f;
	float secondHealthThreshold = 0.35f;

	void GetPositionVariables();
	float distanceToPlayer = 0.0f;
	math::float3 playerPosition = math::float3::zero;
	math::float3 currentPosition = math::float3::zero;
	math::Quat currentRotation = math::Quat::identity;

	float activationDistance = 800.0f;
	float doorClosingDistance = 2000.0f;
	float activationTime = 5.0f;

	bool circlesSpawning = false;
	bool bossTeleporting = false;
	bool bossSummoning = false;
	bool bossExplosives = false;

	//Cutscene state variables

	math::float3 cameraPositionDoorCS = math::float3::zero;
	math::float3 cameraRotationDoorEulerCS = math::float3::zero;
	math::Quat cameraRotationDoorCS = math::Quat::identity;
	math::float3 cameraPositionBossCS = math::float3::zero;
	math::float3 cameraRotationBossEulerCS = math::float3::zero;
	math::Quat cameraRotationBossCS = math::Quat::identity;

	float cutsceneDoorDuration = 10.0f;
	float cutsceneBossDuration = 10.0f;
	float cutsceneBackToPlayerDuration = 10.0f;
	float cutsceneDoorRisingDuration = 2.0f;

	//first cutscene
	math::float3 startingPoint = math::float3::zero;
	math::float3 highPointFirstCS = math::float3::zero;
	math::float3 pointStartFight = math::float3::zero;

	GameObject* closingDoor = nullptr;
	GameObject* doorParticles = nullptr;
	float finalDoorHeight = 600.0f;
	GameObject* playerCamera = nullptr;

	//TP points
	math::float3 topTP = math::float3::zero;
	math::float3 bottomTP = math::float3::zero;
	math::float3 leftTP = math::float3::zero;
	math::float3 rightTP = math::float3::zero;

	//Summon in summon phase
	int summonSkeletonsNumber = 20;
	float timerBetweenSummons = 4.0f;
	math::float3 firstSpawnLocation = math::float3::zero;
	math::float3 secondSpawnLocation = math::float3::zero;

	//Skulls in second phase
	int numberSkullsSecondTotal = 8;
	float timeBetweenSkullsSecond = 0.0f;
	math::float3 northSecondSkull = math::float3::zero;
	math::float3 northEastSecondSkull = math::float3::zero;
	math::float3 eastSecondSkull = math::float3::zero;
	math::float3 southEastSecondSkull = math::float3::zero;
	math::float3 southSecondSkull = math::float3::zero;
	math::float3 southWestSecondSkull = math::float3::zero;
	math::float3 westSecondSkull = math::float3::zero;
	math::float3 northWestSecondSkull = math::float3::zero;
	std::vector<math::float3> positionsSkullsSecond;
	int positionsIt = 0;

public:
		math::Quat InterpolateQuat(const math::Quat first, const math::Quat second, float lambda);
		math::float3 InterpolateFloat3(const math::float3 first, const math::float3 second, float lambda);

private:
	std::vector<BossState*> bossStates;

	void CheckStates(BossState* previous);
	void CheckHealth();
	void FloatInSpace();
	void HandleSkills();

	void HandleFirstTP();
	void HandleSecondTP();
	
	TPlocations ChooseNextTP(TPlocations currentLoc);
	void TPtoLocation(TPlocations tpLoc);

	void HandleFirstPhaseCircles();
	void HandleSecondPhaseCircles();
	void HandleThirdPhaseCircles();
	
	void HandleFirstSummons();
	void HandleSecondSummons();
	void HandleThirdSummons();

	void HandleFirstBombs();
	void HandleSecondBombs();
	void HandleThirdBombs();

	int circlesInFirstPhase = 3;
	float timeBetweenCirclesFirst = 2.0f;
	int circlesInSecondPhase = 4;
	float timeBetweenCirclesSecond = 1.5f;
	int circlesInThirdPhase = 4;
	float timeBetweenCirclesThird = 0.8f;
	float circlesTimer = 0.0f;
	float circleNoise = 5.0f;
	int circlesCast = 0;

	int skullsToShootFirst = 5;
	int numberSkullsShot = 0;
	float skullsTimer = 0.0f;
	float timeBetweenSkulls = 0.2f;
	bool tpPositionDecided = false;
	bool fadeOutComplete = false;
	bool fadeInComplete = false;
	bool skullsShot = false;
	TPState teleportState = TPState::None;

	float timeBetweenBombsFirst = 0.5f;
	int totalBombsFirst = 5;
	int currentBombsThrown = 0;

	bool isFloating = true;
	float angleConstant = 1.0f;
	float floatConstant = 1.0f;
	float yOffset = 0.0f;
	float angle = 0.0f;
};

#endif __BossBehaviourScript_h__
