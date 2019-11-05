#include "BossStateThirdDeath.h"

#include "BossBehaviourScript.h"

#include "Application.h"
#include "ModuleTime.h"

#include "GameObject.h"
#include "ComponentRenderer.h"
#include "ComponentAudioSource.h"

#include "CameraController/CameraController.h"
#include "EnemyControllerScript/EnemyControllerScript.h"
#include "GameLoop/GameLoop.h"
#include "ComponentAnimation.h"

BossStateThirdDeath::BossStateThirdDeath(BossBehaviourScript* AIBoss)
{
	boss = AIBoss;
	trigger = "ThirdDeath";
}


BossStateThirdDeath::~BossStateThirdDeath()
{
}

void BossStateThirdDeath::HandleIA()
{

}

void BossStateThirdDeath::Update()
{
	//superdead, allegedly
	ComponentRenderer* renderer = boss->enemyController->GetMainRenderer();
	
	if (!musicStop)
	{
		float lambda = timer / (duration * 0.8f);
		if (lambda > 1.0f)
		{
			musicStop = true;
			boss->bossBGMusic->Stop();
		}
		else
		{
			boss->bossBGMusic->SetVolume(initialVolume * (1.0f - lambda));
		}
	}

	if (renderer->dissolveAmount >= 1.0f)
	{
		dissolveComplete = true;
	}
	else
	{
		renderer->dissolveAmount += boss->App->time->gameDeltaTime * boss->deathDissolveSpeed;
	}

	if (dissolveComplete && timer > duration)
	{
		
		boss->gLoop->bossDeath = true;
	}
}

void BossStateThirdDeath::Enter()
{
	boss->enemyController->anim->SendTriggerToStateMachine(trigger.c_str());
	duration = boss->anim->GetDurationFromClip();
	boss->cameraScript->Shake(5.0f, 85.0f, 1.0f, 0.01f, false);
	boss->enemyController->bossFightStarted = false;
	initialVolume = boss->bossBGMusic->GetVolume();
	boss->ResetVariables();

	//tell controller that third phase boss is gone
	boss->enemyController->ThirdStageBoss = false;
	boss->EndThirdPhase();
}

void BossStateThirdDeath::Exit()
{
}
