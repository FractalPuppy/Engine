#include "Application.h"
#include "ModuleTime.h"
#include "ModuleScene.h"

#include "GameObject.h"
#include "ComponentRenderer.h"
#include "ComponentTransform.h"
#include "ComponentBoxTrigger.h"
#include "ComponentAudioSource.h"

#include "BossStateInterPhase.h"
#include "CameraController/CameraController.h"

#include "BossBehaviourScript.h"
#include "EnemyControllerScript/EnemyControllerScript.h"
#include "ComponentAnimation.h"

BossStateInterPhase::BossStateInterPhase(BossBehaviourScript* AIBoss)
{
	boss = AIBoss;

	ballRenderer = boss->powerUpBall->GetComponent<ComponentRenderer>();
	ballRenderer->dissolve = true;
	ballRenderer->dissolveAmount = 1.0f;
	originalScale = boss->powerUpBall->transform->scale;
}

BossStateInterPhase::~BossStateInterPhase()
{
}

void BossStateInterPhase::HandleIA()
{
	if (finished)
	{
		boss->currentState = (BossState*)boss->idle;
	}
}

void BossStateInterPhase::Update()
{
	//while getting to the ground, she cannot be attacked
	//but only if the target is the boss, not an skeleton and stuff
	if (boss->App->scene->enemyHovered.object == boss->gameobject)
	{
		boss->App->scene->enemyHovered.object = nullptr;
		boss->App->scene->enemyHovered.health = 0;
		boss->App->scene->enemyHovered.triggerboxMinWidth = 0;
	}
	switch (ipState)
	{
	case IpState::None:
		ipState = IpState::Fadeoff;
		break;
	case IpState::Fadeoff:

		if (boss->enemyController->GetMainRenderer()->dissolveAmount >= 1.0f)
		{
			ipState = IpState::TP;
		}
		else
		{
			for (auto render : boss->enemyController->myRenders)
			{
				if (render != ballRenderer)
				{
					render->dissolveAmount += boss->App->time->gameDeltaTime;
				}
			}
		}

		break;
	case IpState::TP:

		//move her to the throne
		boss->enemyController->SetPosition(boss->firstInterphasePosition);
		boss->enemyController->LookAt2D(boss->pointToLookAtFirstInterphase);
		ipState = IpState::FadeIn;

		break;
	case IpState::FadeIn:

		if (boss->enemyController->GetMainRenderer()->dissolveAmount <= 0.0f)
		{
			ipState = IpState::Powerup;
		}
		else
		{
			for (auto render : boss->enemyController->myRenders)
			{
				if (render != ballRenderer)
				{
					render->dissolveAmount -= boss->App->time->gameDeltaTime;
				}
			}
		}
		boss->enemyController->LookAt2D(boss->pointToLookAtFirstInterphase);

		break;
	case IpState::Powerup:
	{
		if (!durationPowerSet)
		{
			boss->anim->SendTriggerToStateMachine("PowerUp");
			boss->firstInterphaseDuration = boss->anim->GetDurationFromClip();
			durationPowerSet = true;
		}

		if (!fxHandBallSet && powerUpTimer > boss->handsAppearTime)
		{
			boss->rightHandBall->SetActive(true);
			boss->leftHandBall->SetActive(true);
			fxHandBallSet = true;
		}

		if (!fxBigBallSet && powerUpTimer > boss->ballAppearTime)
		{
			fxState = FX::Charge;
			fxBigBallSet = true;
		}

		if (powerUpTimer > boss->firstInterphaseDuration)
		{
			ipState = IpState::Relocate;
			boss->anim->SendTriggerToStateMachine("Idle");
		}
		else
		{
			powerUpTimer += boss->App->time->gameDeltaTime;
		}
		boss->enemyController->LookAt2D(boss->pointToLookAtFirstInterphase);

		float lambdaBis = 3 * powerUpTimer / boss->firstInterphaseDuration;
		if (lambdaBis < 1.0f)
		{
			boss->enemyController->SetPosition(boss->InterpolateFloat3(boss->firstInterphasePosition, boss->firstInterphasePowerUpPosition, lambdaBis));
		}
	}
		break;

	case IpState::Relocate:
	{
		//better to move and not to TP
		relocateTimer += boss->App->time->gameDeltaTime;

		float lambda = relocateTimer / boss->relocateInterPhaseTime;

		boss->enemyController->SetPosition(boss->InterpolateFloat3(boss->firstInterphasePowerUpPosition, boss->topTP, lambda));

		if (lambda >= 1.0f)
		{
			ipState = IpState::Finished;
		}
		boss->enemyController->LookAt2D(boss->pointToLookAtFirstInterphase);
	}
		break;

	case IpState::Finished:
		finished = true;
		break;
	}

	if (fxState != FX::None)
	{
		fxTimer += boss->App->time->gameDeltaTime;
		if (fxTimer > boss->ballFxStopTime)
		{
			boss->powerUpSpread->SetActive(false);
			boss->powerUpBall->SetActive(false);
		}
		if (fxTimer > 0.5f + boss->ballExplodeTime)
		{
			boss->ringPowerUp->SetActive(false);
		}
	}

	switch (fxState)
	{
	case FX::None:
		break;
	case FX::Charge:
		
		if (ballRenderer->dissolveAmount <= 0.0f)
		{
			fxState = FX::Loop;
			boss->rightHandBall->SetActive(false);
			boss->leftHandBall->SetActive(false);
		}
		else
		{
			ballRenderer->dissolveAmount -= boss->App->time->gameDeltaTime;
		}

		break;
	case FX::Loop:

		//Here we can make it vibrate a bit
		angle += boss->vibrationSpeed * boss->App->time->gameDeltaTime;
		offset = boss->vibrationAmplitude * sin(angle);

		boss->powerUpBall->transform->scale = originalScale + offset * math::float3::one;
		
		if (fxTimer > boss->ballExplodeTime)
		{
			fxState = FX::Explode;
			boss->ringPowerUp->SetActive(true);
			boss->powerUpSpread->SetActive(true);
			boss->bossScream->Play();
			boss->cameraScript->Shake(1.5f, 100.0f, 0.1f, 0.8f, false);
			break;
		}

		break;
	case FX::Explode:	

		//Here we cativate both ring and spread aprticles and dissolve the sphere
		if (ballRenderer->dissolveAmount >= 1.0f)
		{
			fxState = FX::Finished;
		}
		else
		{
			ballRenderer->dissolveAmount += 2.0f * boss->App->time->gameDeltaTime;
		}

		break;
	case FX::Finished:
		break;
	}

}

void BossStateInterPhase::Enter()
{
	//Here we reset all bosses variables for the next phase
	boss->ResetVariables();
	boss->enemyController->hpBoxTrigger->Enable(false);
}

void BossStateInterPhase::Exit()
{
	boss->enemyController->hpBoxTrigger->Enable(true);
}
