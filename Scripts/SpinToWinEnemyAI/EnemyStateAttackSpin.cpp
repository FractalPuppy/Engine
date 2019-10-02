#include "EnemyStateAttackSpin.h"

#include "GameObject.h"

#include "ComponentTransform.h"
#include "ComponentBoxTrigger.h"
#include "ComponentRenderer.h"
#include "ComponentTrail.h"

#include "ResourceMaterial.h"
#include "ResourceTexture.h"

#include "BasicEnemyAIScript/BasicEnemyAIScript.h"
#include "EnemyControllerScript/EnemyControllerScript.h"
#include "SpinToWinEnemyAI.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleTime.h"
#include "ModuleResourceManager.h"

#include "ComponentTrail.h"

EnemyStateAttackSpin::EnemyStateAttackSpin(BasicEnemyAIScript* AIScript) : EnemyStateAttack(AIScript)
{
	trigger = "Attack";
	enemyRenderer = (ComponentRenderer*)enemy->gameobject->GetComponentInChildren(ComponentType::Renderer);
	dust = enemy->App->scene->FindGameObjectByName("Dust", enemy->gameobject);
	spinOff = enemy->App->scene->FindGameObjectByName("SpinLines", enemy->gameobject);
}

EnemyStateAttackSpin::~EnemyStateAttackSpin()
{
}

void EnemyStateAttackSpin::HandleIA() //Should check spin 75% prob & heatlh < 50% + timer to do it again (x seconds)
{
	//If player is in range nothing, else if player is in range chase, if enemy has attacked cooldown
	
	if (timer > duration)
	{
		float distance = enemy->enemyController->GetDistanceToPlayer2D();
		if (distance > enemy->attackRange && !spinning)
		{
			if (hitboxCreated)
			{
				// Disable hitbox
				enemy->enemyController->attackBoxTrigger->Enable(false);
				hitboxCreated = false;
			}
			PunchFX(false);
			enemy->currentState = (EnemyState*)enemy->chase;
		}
		else if (attacked)
		{
			if (hitboxCreated)
			{
				// Disable hitbox
				enemy->enemyController->attackBoxTrigger->Enable(false);
				hitboxCreated = false;
			}
			enemy->currentState = (EnemyState*)enemy->cooldown;
			attacked = false;
			PunchFX(false);
		}
	}

}

void EnemyStateAttackSpin::Update()
{
	// Keep looking at player
	assert(enemy->enemyController->attackBoxTrigger != nullptr);

	Attack();
}

void EnemyStateAttackSpin::Enter()
{
}

void EnemyStateAttackSpin::Exit()
{
	if (spinning) //Reset spinning behaviour
	{
		DisableSpin();
	}
}

void EnemyStateAttackSpin::Attack() //Split into SPIN or normal ATTACK
{

	if ((!LessThanHalfHP() || lcg.Float() < 0.25 || isOnCooldown) && !spinning) //Normal attack
	{
		enemy->enemyController->attackBoxTrigger->Enable(true);
		enemy->enemyController->attackBoxTrigger->SetBoxSize(boxSize);
		boxPosition = enemy->gameobject->transform->up * 100.f;
		enemy->enemyController->attackBoxTrigger->SetBoxPosition(boxPosition.x, boxPosition.y, boxPosition.z + 100.f);
		hitboxCreated = true;
		attacked = true;
		spinTimer += enemy->App->time->gameDeltaTime; //Spin cooldown
		if (spinTimer > spinCooldown)
		{
			isOnCooldown = false;
			spinTimer = 0.0f;
		}
		PunchFX(true);
	}
	else if(spinTimer < spinDuration)
	{
		SpinBehaviour();
	}
	else
	{
		DisableSpin();
	}
}

bool EnemyStateAttackSpin::LessThanHalfHP() const
{
	return enemy->enemyController->GetHealth() <= enemy->enemyController->GetMaxHealth() * 0.8f;
}

void EnemyStateAttackSpin::PunchFX(bool active)
{
	EnemyStateAttack::PunchFX(active);

	if (spinning && active)
	{
		trailPunch->width = 40.0f;
		dust->SetActive(true);
		SpinBones(false);
	}
	else
	{
		trailPunch->width = 25.0f;
		dust->SetActive(false);
		SpinBones(true);
	}
}

void EnemyStateAttackSpin::ChangeToSpinMaterial(MATERIALTYPE type) const
{
	switch (type)
	{
	case MATERIALTYPE::ROTATION:
		enemyRenderer->material = rotationMaterial;
		break;
	case MATERIALTYPE::DEFAULT:
		enemyRenderer->material = defaultMaterial;
		break;
	}

	enemy->gameobject->LinkBones();
}

void EnemyStateAttackSpin::EnableSpin()
{
	spinning = true;
	PunchFX(true);
	attacked = false;
	enemyRenderer->avoidSkinning = true;
}

void EnemyStateAttackSpin::DisableSpin()
{
	PunchFX(false);
	enemyRenderer->avoidSkinning = false;
	spinOff->SetActive(false);
	spinTimer = 0.0f;
	spinning = false;
	isOnCooldown = true;
	attacked = true;
	math::float3 playerPosition = enemy->enemyController->GetPlayerPosition();
	enemy->enemyController->LookAt2D(playerPosition);
}

void EnemyStateAttackSpin::SpinBehaviour()
{
	if (!spinning)
	{
		EnableSpin();
	}
	spinTimer += enemy->App->time->gameDeltaTime;
	if (spinTimer >= 0.75f * spinDuration && !spinOff->isActive())
	{
		spinOff->SetActive(true);
	}
	float distance = enemy->enemyController->GetDistanceToPlayer2D();
	if (distance > enemy->attackRange)
	{
		math::Quat currentRotation = enemy->gameobject->transform->GetRotation();
		enemy->enemyController->Move(enemy->chaseSpeed*1.5f, refreshTime, enemy->enemyController->GetPlayerPosition(), enemyPath);
		enemy->gameobject->transform->SetRotation(currentRotation);
	}
	enemy->gameobject->transform->Rotate(math::float3(0.f, enemy->App->time->gameDeltaTime * rotationSpeed, 0.f));
}

void EnemyStateAttackSpin::SpinBones(bool active)
{
	for (auto spinBone : ((SpinToWinEnemyAI*)enemy)->spinBones)
	{
		spinBone->SetActive(active);
	}
}
