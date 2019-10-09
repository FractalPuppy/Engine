#include "Application.h"
#include "ModuleTime.h"

#include "ComponentRenderer.h"
#include "ComponentTransform.h"
#include "ComponentBoxTrigger.h"

#include "BossStateInterPhase.h"

#include "BossBehaviourScript.h"
#include "EnemyControllerScript/EnemyControllerScript.h"
#include "ComponentAnimation.h"

BossStateInterPhase::BossStateInterPhase(BossBehaviourScript* AIBoss)
{
	boss = AIBoss;
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
				render->dissolveAmount += boss->App->time->gameDeltaTime;
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
				render->dissolveAmount -= boss->App->time->gameDeltaTime;
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


		//this could be better animation driven, but w/e
		if (powerUpTimer > boss->firstInterphaseDuration)
		{
			ipState = IpState::Relocate;
			boss->anim->SendTriggerToStateMachine("Idle");
			durationPowerSet = false;
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
