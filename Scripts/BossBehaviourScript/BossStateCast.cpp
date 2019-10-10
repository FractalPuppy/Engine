#include "Application.h"
#include "ModuleTime.h"
#include "ModuleScene.h"
#include "GameObject.h"

#include "BossStateCast.h"

#include "ComponentAnimation.h"

#include "BossBehaviourScript.h"
#include "EnemyControllerScript/EnemyControllerScript.h"

BossStateCast::BossStateCast(BossBehaviourScript* AIBoss)
{
	boss = AIBoss;
	trigger = "Cast";
}


BossStateCast::~BossStateCast()
{
}

void BossStateCast::HandleIA()
{
	if (timer > duration)
	{
		boss->currentState = (BossState*)boss->idle;
	}
}

void BossStateCast::Update()
{
	boss->enemyController->LookAt2D(boss->playerPosition);

	if (!orbsUnset)
	{
		if (orbsTimer / duration > boss->percOrbsDisappear)
		{
			boss->leftHandBall->SetActive(false);
			boss->rightHandBall->SetActive(false);
			boss->leftHandParticles->SetActive(false);
			boss->rightHandParticles->SetActive(false);
			orbsUnset = true;
		}
		else
		{
			orbsTimer += boss->App->time->gameDeltaTime;
		}
	}
}

void BossStateCast::Exit()
{

	BossSkill nextSkill = SelectSkillToUse();

	switch (nextSkill)
	{
	case BossSkill::Aoe:
		boss->circlesSpawning = true;
		break;
	case BossSkill::Teleport:
		boss->bossTeleporting = true;
		break;
	case BossSkill::Summon:
		boss->bossSummoning = true;
		break;
	case BossSkill::Bombs:
		boss->bossExplosives = true;
		break;
	}

	orbsUnset = false;
	orbsTimer = 0.0f;
}

void BossStateCast::Enter()
{
	//I can input here different durations for each skill

	boss->anim->SendTriggerToStateMachine(trigger.c_str());
	duration = boss->anim->GetDurationFromClip();
}

BossSkill BossStateCast::SelectSkillToUse()
{
	int randomNumb = rand() % 2;

	BossSkill skillToCast = boss->lastUsedSkill;

	while (skillToCast == boss->lastUsedSkill || skillToCast == boss->secondLastUsedSkill)
	{
		int randomNumb = rand() % 3;

		switch (randomNumb)
		{
		case 0:
			skillToCast = BossSkill::Teleport;
			break;
		case 1:
			skillToCast = BossSkill::Summon;
			break;
		case 2:
			skillToCast = BossSkill::Aoe;
			break;
		}
	}

	boss->secondLastUsedSkill = boss->lastUsedSkill;
	boss->lastUsedSkill = skillToCast;

	return skillToCast;
}
