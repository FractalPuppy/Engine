#include "PlayerMovement.h"

#include "Application.h"

#include "ModuleInput.h"
#include "ModuleScene.h"
#include "ModuleNavigation.h"
#include "ModuleTime.h"
#include "ModuleWindow.h"

#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentAnimation.h"

#include "PlayerStateWalkToHitEnemy.h"
#include "PlayerStateIdle.h"

#include "BasicSkill.h"

#include "JSON.h"
#include <assert.h>
#include <string>
#include "imgui.h"
#include "Globals.h"
#include "debugdraw.h"

#define RECALC_PATH_TIME 0.3f

PlayerStateWalkToHitEnemy::PlayerStateWalkToHitEnemy(PlayerMovement* PM, const char* trigger):
	PlayerState(PM, trigger)
{
}

PlayerStateWalkToHitEnemy::~PlayerStateWalkToHitEnemy()
{
}

void PlayerStateWalkToHitEnemy::Update()
{
	//enemy targeting check
	if (walkingEnemyTargeted == nullptr || (player->App->scene->enemyHovered.object != nullptr && player->App->scene->enemyHovered.object != walkingEnemyTargeted))
	{
		if (player->App->scene->enemyHovered.object != nullptr)
		{
			walkingEnemyTargeted = player->App->scene->enemyHovered.object;
			enemyPosition = walkingEnemyTargeted->transform->position;
			math::float3 correctionPos(player->basicAttackRange, player->OutOfMeshCorrectionY, player->basicAttackRange);
			if (player->App->navigation->FindPath(player->gameobject->transform->position, enemyPosition,
				path, PathFindType::FOLLOW, correctionPos, defaultMaxDist, player->straightPathingDistance))
			{
				//case the player clicks outside of the floor mesh but we want to get close to the floors edge
				pathIndex = 0;
			}
		}
		else
		{
			//something went wrong, stop moving
			LOG("Error walking to hit enemy");
			playerWalking = false;
			return;
		}
	}

	//if enemy moved, re calc path towards it
	if (walkingEnemyTargeted->transform->position != enemyPosition)
	{
		//reset stuff
		path.clear();
		enemyPosition = walkingEnemyTargeted->transform->position;
		//re calculate path
		if (player->App->navigation->FindPath(player->gameobject->transform->position, enemyPosition,
			path, PathFindType::FOLLOW, math::float3(player->basicAttackRange, player->OutOfMeshCorrectionY, player->basicAttackRange),
			defaultMaxDist, player->straightPathingDistance))
		{
			pathIndex = 0;
		}
		//if error
		else
		{
			LOG("Error walking to hit enemy along the way");
			playerWalking = false;
			return;
		}
	}
	if (path.size() > 0)
	{
		math::float3 currentPosition = player->gameobject->transform->GetPosition();
		while (pathIndex < path.size() && currentPosition.DistanceSq(path[pathIndex]) < MINIMUM_PATH_DISTANCE)
		{
			pathIndex++;
		}
		if (pathIndex < path.size() && player->basicAttackRange < Distance(player->gameobject->transform->position, enemyPosition))
		{
			player->gameobject->transform->LookAt(path[pathIndex]);
			math::float3 direction = (path[pathIndex] - currentPosition).Normalized();
			//lerping if necessary
			lerpCalculations(direction, -player->gameobject->transform->front, path[pathIndex]);

			math::float3 finalWalkingSpeed = player->walkingSpeed * direction * player->App->time->gameDeltaTime;
			finalWalkingSpeed *= (1 + (player->stats.dexterity * 0.005f));
			player->gameobject->transform->SetPosition(currentPosition + finalWalkingSpeed);
			playerWalking = true;
			playerWalkingToHit = true;
			if (dustParticles)
			{
				dustParticles->SetActive(true);
			}
		}
		else
		{
			toAttack = true;
		}
	}
	else
	{
		playerWalking = false;
	}
}

void PlayerStateWalkToHitEnemy::Enter()
{
	toAttack = false;
	if (dustParticles)
	{
		dustParticles->SetActive(true);
		player->anim->controller->current->speed *= (1 + (player->stats.dexterity * 0.005f));
	}
}

void PlayerStateWalkToHitEnemy::CheckInput()
{
	if (!playerWalking)
	{
		playerWalkingToHit = false;
		player->currentState = player->idle;
		if (dustParticles)
		{
			dustParticles->SetActive(false);
		}
		return;
	}
	if (toAttack)
	{
		//done walking, lets hit the enemy
		//about the orientation of the player, in the chain attack state looks at the mouse automatically
		player->enemyTargeted = true;
		player->enemyTarget = walkingEnemyTargeted;

		playerWalkingToHit = false;
		playerWalking = false;

		player->currentSkill = player->allSkills[SkillType::CHAIN]->skill;

		SkillType skillType = SkillType::CHAIN;

		//entering code
		{

			player->currentState = (PlayerState*)player->attack;

			// Play skill animation
			if (player->anim != nullptr)
			{
				player->anim->SendTriggerToStateMachine(player->currentSkill->animTrigger.c_str());
			}

			player->currentSkill->duration = player->anim->GetDurationFromClip();

			player->UseSkill(skillType);
			player->currentSkill->Start();

		}
		if (dustParticles)
		{
			dustParticles->SetActive(false);
		}
		toAttack = false;
		return;
	}
	if (player->IsUsingSkill() || (player->IsAttacking()))
	{
		player->currentState = (PlayerState*)player->attack;
	}
	else if (player->IsMovingToAttack())
	{
		player->currentState = (PlayerState*)player->walkToHit;
	}
	else if (player->IsMovingToItem())
	{
		player->currentState = (PlayerState*)player->walkToPickItem;
	}
	else if (player->IsMoving())
	{
		player->currentState = (PlayerState*)player->walk;
	}
	else if(!playerWalkingToHit)
	{
		player->currentState = (PlayerState*)player->idle;
		if (dustParticles)
		{
			dustParticles->SetActive(false);
		}
	}
}