#include "Application.h"

#include "ModuleTime.h"

#include "BossStateCutScene.h"

#include "BossBehaviourScript.h"
#include "PlayerMovement/PlayerMovement.h"
#include "CameraController/CameraController.h"
#include "EnemyControllerScript/EnemyControllerScript.h"

#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentAnimation.h"

BossStateCutScene::BossStateCutScene(BossBehaviourScript* AIBoss)
{
	boss = AIBoss;
}

BossStateCutScene::~BossStateCutScene()
{
}

void BossStateCutScene::HandleIA()
{
	//if finished everything change to summon army
	if (finished)
	{
		boss->currentState = (BossState*)boss->summonArmy;
	}
}

void BossStateCutScene::Update()
{
	switch (csState)
	{
		case CutsceneState::None:
			csState = CutsceneState::DoorLerping;
			break;
		case CutsceneState::DoorLerping:

			if (doorLerpTimer >= boss->cutsceneDoorDuration)
			{
					csState = CutsceneState::DoorClosing;
					boss->doorParticles->SetActive(true);
					wallSpeed = (boss->finalDoorHeight - boss->closingDoor->transform->GetPosition().y) / boss->cutsceneDoorRisingDuration;
			}
			else
			{
				doorLerpTimer += boss->App->time->gameDeltaTime;

				firstLambda = CalculateDoorLambda();
				SetPlayerCameraPosition(boss->InterpolateFloat3(cameraResetPosition, boss->cameraPositionDoorCS, firstLambda)); 
				SetPlayerCameraRotation(boss->InterpolateQuat(cameraResetRotation, boss->cameraRotationDoorCS, firstLambda));
			}
			break;

		case CutsceneState::DoorClosing:
		{
			doorClosingTimer += boss->App->time->gameDeltaTime;

			math::float3 newDoorPosition = boss->closingDoor->transform->GetPosition() + wallSpeed * math::float3::unitY * boss->App->time->gameDeltaTime;
			boss->closingDoor->transform->SetPosition(newDoorPosition);

			if (doorClosingTimer >= boss->cutsceneDoorRisingDuration)
			{
				boss->doorParticles->SetActive(false);
				csState = CutsceneState::BossLerping;
			}
			break;
		}
		case CutsceneState::BossLerping:

			if (bossLerpTimer >= boss->cutsceneBossDuration)
			{
				csState = CutsceneState::BossWatching;
			}
			else
			{
				bossLerpTimer += boss->App->time->gameDeltaTime;

				secondLambda = CalculateBossLambda();
				SetPlayerCameraPosition(boss->InterpolateFloat3(boss->cameraPositionDoorCS, boss->cameraPositionBossCS, secondLambda));
				SetPlayerCameraRotation(boss->InterpolateQuat(boss->cameraRotationDoorCS, boss->cameraRotationBossCS, secondLambda));
			}
			break;
		case CutsceneState::BossWatching:

			if (bossWatchingTimer >= 4.0f)
			{
				csState = CutsceneState::PlayerLerping;
			}
			else
			{
				bossWatchingTimer += boss->App->time->gameDeltaTime;
			}
			break;
		case CutsceneState::PlayerLerping:

			if (playerLerpTimer >= boss->cutsceneBackToPlayerDuration)
			{
				csState = CutsceneState::Finished;
			}
			else
			{
				playerLerpTimer += boss->App->time->gameDeltaTime;

				thirdLambda = CalculatePlayerLambda();
				SetPlayerCameraPosition(boss->InterpolateFloat3(boss->cameraPositionBossCS, cameraResetPosition, thirdLambda));
				SetPlayerCameraRotation(boss->InterpolateQuat(boss->cameraRotationBossCS, cameraResetRotation, thirdLambda));

			}
			break;

		case CutsceneState::Finished:
			finished = true;
			break;
	}
	//flames?

	//do the lerping in

	//Close door

	//lerping to boss

	//back to player

}

void BossStateCutScene::Enter()
{
	cameraResetPosition = GetPlayerCameraPosition();
	cameraResetRotation = GetPlayerCameraRotation();

	firstCameraDirection = boss->cameraPositionDoorCS - cameraResetPosition;
	doorDistance = firstCameraDirection.Length();
	firstCameraSpeed = doorDistance / boss->cutsceneDoorDuration;
	firstCameraDirection.Normalize();

	secondCameraDirection = boss->cameraPositionBossCS - boss->cameraPositionDoorCS;
	bossDistance = secondCameraDirection.Length();
	secondCameraSpeed = bossDistance / boss->cutsceneBossDuration;
	secondCameraDirection.Normalize();

	//Deactivate player script
	boss->playerScript->anim->SendTriggerToStateMachine("Idle");
	boss->playerScript->Enable(false);
	//Deactivate camera script
	boss->playerCamera->GetComponent<CameraController>()->Enable(false);
}

void BossStateCutScene::Exit()
{
	//Activate player script
	boss->playerScript->Enable(true);
	//Activate camera script
	boss->playerCamera->GetComponent<CameraController>()->Enable(true);
}

float BossStateCutScene::CalculateDoorLambda()
{
	return (doorLerpTimer + boss->App->time->gameDeltaTime) / boss->cutsceneDoorDuration;
}

float BossStateCutScene::CalculateBossLambda()
{
	return (bossLerpTimer + boss->App->time->gameDeltaTime) / boss->cutsceneBossDuration;
}

float BossStateCutScene::CalculatePlayerLambda()
{
	return (playerLerpTimer + boss->App->time->gameDeltaTime) / boss->cutsceneBackToPlayerDuration;
}

math::float3 BossStateCutScene::GetPlayerCameraPosition()
{
	return boss->playerCamera->transform->GetPosition();
}

math::Quat BossStateCutScene::GetPlayerCameraRotation()
{
	return boss->playerCamera->transform->GetRotation();
}

void BossStateCutScene::SetPlayerCameraPosition(math::float3 newPosition)
{
	boss->playerCamera->transform->SetPosition(newPosition);
}

void BossStateCutScene::SetPlayerCameraRotation(math::Quat newRotation)
{
	boss->playerCamera->transform->SetRotation(newRotation);
}