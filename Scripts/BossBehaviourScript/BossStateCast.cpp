#include "Application.h"
#include "ModuleTime.h"
#include "ModuleScene.h"

#include "BossStateCast.h"

#include "BossBehaviourScript.h"
#include "EnemyControllerScript/EnemyControllerScript.h"

BossStateCast::BossStateCast(BossBehaviourScript* AIBoss)
{
	boss = AIBoss;
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
}

void BossStateCast::Enter()
{
	
	BossSkill nextSkill = SelectSkillToUse();

	switch (nextSkill)
	{
	case BossSkill::Aoe:
		boss->circlesSpawning = true;
		duration = 1.0f;
		break;
	case BossSkill::Teleport:
		boss->bossTeleporting = true;
		duration = 5.0f;
		break;
	case BossSkill::Summon:
		duration = 2.0f;
		boss->bossSummoning = true;
		break;
	case BossSkill::Bombs:
		duration = 2.0f;
		boss->bossExplosives = true;
		break;
	}
	//I can input here different durations for each skill
}

BossSkill BossStateCast::SelectSkillToUse()
{
	int randomNumb = rand() % 2;

	BossSkill skillToCast = boss->lastUsedSkill;

	while (skillToCast == boss->lastUsedSkill || skillToCast == boss->secondLastUsedSkill)
	{
		int randomNumb = rand() % 4;

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
		case 3:
			skillToCast = BossSkill::Bombs;
			break;
		}
	}

	boss->secondLastUsedSkill = boss->lastUsedSkill;
	boss->lastUsedSkill = skillToCast;

	return skillToCast;
}
