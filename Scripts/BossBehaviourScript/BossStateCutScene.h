#ifndef _BOSSSTATECUTSCENE_H_
#define _BOSSSTATECUTSCENE_H_

#include "BossState.h"
#include "Math/float3.h"
#include "Math/Quat.h"

enum class CutsceneState
{
	None,
	DoorLerping,
	DoorClosing,
	BossLerping,
	BossWatching,
	PlayerLerping,
	Finished
};

class BossStateCutScene :
	public BossState
{
public:
	BossStateCutScene(BossBehaviourScript* AIBoss);
	~BossStateCutScene();

private:

	void HandleIA() override;
	void Update() override;

	void Enter() override;
	void Exit() override;

	bool finished = false;

	math::float3 firstCameraDirection = math::float3::zero;
	math::float3 secondCameraDirection = math::float3::zero;
	math::float3 cameraResetPosition = math::float3::zero;
	math::Quat cameraResetRotation = math::Quat::identity;

	float doorDistance = 0.0f;
	float bossDistance = 0.0f;

	float firstCameraSpeed = 0.0f;
	float secondCameraSpeed = 0.0f;

	float doorLerpTimer = 0.0f;
	float doorClosingTimer = 0.0f;
	float bossLerpTimer = 0.0f;
	float bossWatchingTimer = 0.0f;
	float playerLerpTimer = 0.0f;

	float firstLambda = 0.0f;
	float secondLambda = 0.0f;
	float thirdLambda = 0.0f;

	float CalculateDoorLambda();
	float CalculateBossLambda();
	float CalculatePlayerLambda();

	math::float3 GetPlayerCameraPosition();
	math::Quat GetPlayerCameraRotation();
	void SetPlayerCameraPosition(math::float3 newPosition);
	void SetPlayerCameraRotation(math::Quat newRotation);
	
	CutsceneState csState = CutsceneState::None;

private:
	bool particlesShut = false;
	bool doorClosed = false;
	float wallSpeed = 0.0f;
};

#endif