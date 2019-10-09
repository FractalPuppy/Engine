#include "Application.h"
#include "ModuleScene.h"
#include "ModuleTime.h"

#include "GameObject.h"
#include "ComponentRenderer.h"
#include "ComponentBoxTrigger.h"
#include "ComponentAnimation.h"
#include "ComponentTransform.h"

#include "BossStateSecondInterPhase.h"
#include "EnemyControllerScript/EnemyControllerScript.h"
#include "GameLoop/GameLoop.h"

#include "BossBehaviourScript.h"

BossStateSecondInterPhase::BossStateSecondInterPhase(BossBehaviourScript* AIboss)
{
	boss = AIboss;
}


BossStateSecondInterPhase::~BossStateSecondInterPhase()
{
}

void BossStateSecondInterPhase::HandleIA()
{
	//this will have a fixed duration
	if (finished)
	{
		boss->currentState = (BossState*)boss->thirdIdle;
	}
}

void BossStateSecondInterPhase::Update()
{
	//Update parts

	switch (state)
	{
		case InterphaseState::None:
			state = InterphaseState::Kneel;
			boss->enemyController->anim->SendTriggerToStateMachine("Kneel");
			startingKneelY = boss->currentPosition.y;
			boss->secondInterphaseKneelTime = boss->enemyController->anim->GetDurationFromClip();
			break;
		case InterphaseState::Kneel:
			if (kneelTimer > boss->secondInterphaseKneelTime)
			{
				state = InterphaseState::Cry;
				boss->enemyController->anim->SendTriggerToStateMachine("Cry");

			}
			else
			{
				kneelTimer += boss->App->time->gameDeltaTime;
				float lambda = 5 * kneelTimer / boss->secondInterphaseKneelTime;
				if (lambda > 1.0f)
				{
					lambda = 1.0f;
				}
				float newY = boss->InterpolateFloat(startingKneelY, boss->endingHeightKneel, lambda);
				boss->enemyController->SetPosition(math::float3(boss->currentPosition.x, newY, boss->currentPosition.z));
			}
			break;
		case InterphaseState::Cry:
			if (cryTimer > boss->secondInterphaseCryTime)
			{
				state = InterphaseState::FadeOff;
			}
			else
			{
				cryTimer += boss->App->time->gameDeltaTime;
			}
			break;
		case InterphaseState::FadeOff:

			if (fadeOffTimer / boss->secondInterphaseFadeOffTime < 1.0f)
			{
				for (auto render : boss->enemyController->myRenders)
				{
					render->dissolveAmount = fadeOffTimer / boss->secondInterphaseFadeOffTime;
				}	
			}
			else
			{
				state = InterphaseState::FloorVanish;
			}

			fadeOffTimer += boss->App->time->gameDeltaTime;
			break;
		case InterphaseState::FloorVanish:
			//just vanish everything
			//I need to load a vector with all the renders.. not yet finished
			if (floorVanishTimer / boss->secondInterphaseFloorVanishTime < 1.0f)
			{
				for (auto vanishGO : boss->secondInterphaseVanishGOs)
				{
					vanishGO->GetComponent<ComponentRenderer>()->dissolveAmount = floorVanishTimer / boss->secondInterphaseFloorVanishTime;
				}
			}
			else
			{
				state = InterphaseState::Teleport;
			}

			floorVanishTimer += boss->App->time->gameDeltaTime;

			//state = InterphaseState::Teleport;

			break;
		case InterphaseState::Teleport:

			boss->enemyController->SetPosition(boss->secondInterphaseFinalPosition);
			boss->enemyController->LookAt2D(boss->playerPosition);
			boss->gameobject->transform->SetScale(math::float3(boss->secondInterphaseFinalScale, boss->secondInterphaseFinalScale, boss->secondInterphaseFinalScale));
			boss->enemyController->anim->SendTriggerToStateMachine("ThirdIdle");
			state = InterphaseState::FadeIn;
			break;
		case InterphaseState::FadeIn:

			if (fadeInTimer / boss->secondInterphaseFadeInTime < 1.0f)
			{
				for (auto render : boss->enemyController->myRenders)
				{
					render->dissolveAmount = 1 - (fadeInTimer / boss->secondInterphaseFadeInTime);
				}

			}
			else
			{
				state = InterphaseState::Finished;
			}

			fadeInTimer += boss->App->time->gameDeltaTime;
			
			break;
		case InterphaseState::Finished:
			finished = true;
			break;
	}
	//Make her kneel

	//Disappear

	//Throne floor and everything on top needs to disappear. no need to touch the navmesh

	//Teleport her to the designated spot and change her scale

	//change the animation and make her appear again

	//maybe wait a bit idk
}

void BossStateSecondInterPhase::Enter()
{
	//We need to reset variables and get everything ready
	boss->ResetVariables();
	boss->enemyController->hpBoxTrigger->Enable(false);
}

void BossStateSecondInterPhase::Exit()
{
	boss->gLoop->DeleteAllEnemies();
	boss->enemyController->hpBoxTrigger->Enable(true);
}
