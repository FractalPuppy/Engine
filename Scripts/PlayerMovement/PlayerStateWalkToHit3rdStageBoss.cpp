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

#include "PlayerStateWalkToHit3rdStageBoss.h"
#include "PlayerStateIdle.h"

#include "BasicSkill.h"

#include "JSON.h"
#include <assert.h>
#include <string>
#include "imgui.h"
#include "Globals.h"
#include "debugdraw.h"

#define RECALC_PATH_TIME 0.3f

PlayerStateWalkToHit3rdStageBoss::PlayerStateWalkToHit3rdStageBoss(PlayerMovement* PM, const char* trigger):
	PlayerState(PM, trigger)
{
}

PlayerStateWalkToHit3rdStageBoss::~PlayerStateWalkToHit3rdStageBoss()
{
}

void PlayerStateWalkToHit3rdStageBoss::Update()
{
	//check if gotta do another pathfinding call
	currentTime -= player->App->time->gameDeltaTime;
	if (walkingEnemyTargeted == nullptr || (currentTime <= 0.0f || path.size() == 0))
	{
		currentTime = timeBetweenMoveCalls;
		//if we dont have a target yet
		if (player->App->scene->enemyHovered.object)
		{
			walkingEnemyTargeted = player->App->scene->enemyHovered.object->parent;
			enemyPosition = walkingEnemyTargeted->transform->position;
			targetBoxWidth = player->App->scene->enemyHovered.triggerboxMinWidth;
			math::float3 correctionPos(player->basicAttackRange*10, player->OutOfMeshCorrectionY*20, player->basicAttackRange*10);
			if (player->App->navigation->FindPath(player->gameobject->transform->position, enemyPosition,
				path, PathFindType::FOLLOW, correctionPos, defaultMaxDist, player->straightPathingDistance))
			{
				//case the player clicks outside of the floor mesh but we want to get close to the floors edge
				pathIndex = 0;
			}
		}
		//if we dont have any kind of target
		else if(!walkingEnemyTargeted)
		{
			//something went wrong, stop moving
			LOG("Error walking to hit enemy");
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
		math::float2 posPlayer2D = math::float2(player->gameobject->transform->position.x,
												player->gameobject->transform->position.z);
		math::float2 posEnemy2D = math::float2(	enemyPosition.x,
												enemyPosition.z);
		//todo:: check this out, maybe 1st cond is not necessary and 
		//player range = stuff prob has to be hardcoded a 450.0f instead
		//player->App->scene->enemyHovered.triggerboxMinWidth*0.1
		if (pathIndex < path.size() && player->basicAttackRange + targetBoxWidth * 0.2 <=
			Distance(posPlayer2D, posEnemy2D))
		{
			math::float3 lookatPos = math::float3(path[pathIndex].x, player->gameobject->transform->position.y, path[pathIndex].z);
			player->gameobject->transform->LookAt(lookatPos);
			math::float3 direction = (path[pathIndex] - currentPosition).Normalized();
			//lerping if necessary
			lerpCalculations(direction, -player->gameobject->transform->front, path[pathIndex]);

			math::float3 finalWalkingSpeed = player->walkingSpeed * direction * player->App->time->gameDeltaTime;
			finalWalkingSpeed *= (1 + (player->GetTotalPlayerStats().dexterity * 0.005f));
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
			path.clear();
		}
	}
	else
	{
		path.clear();
		playerWalking = false;
	}
}

void PlayerStateWalkToHit3rdStageBoss::Enter()
{
	toAttack = false;
	if (dustParticles)
	{
		dustParticles->SetActive(true);
		player->anim->controller->current->speed *= (1 + (player->GetTotalPlayerStats().dexterity * 0.005f));
	}
}

void PlayerStateWalkToHit3rdStageBoss::CheckInput()
{
	if (!playerWalking)
	{
		path.clear();
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
		path.clear();
		player->currentState = (PlayerState*)player->attack;
	}
	else if (player->IsMovingToAttack())
	{
		if (player->ThirdStageBoss)
		{
			player->currentState = (PlayerState*)player->walkToHit3rdBoss;
		}
		else
		{
			player->currentState = (PlayerState*)player->walkToHit;
		}
	}
	else if (player->IsMovingToItem())
	{
		path.clear();
		player->currentState = (PlayerState*)player->walkToPickItem;
	}
	else if (player->IsMoving())
	{
		path.clear();
		player->currentState = (PlayerState*)player->walk;
	}
	else if(!playerWalkingToHit)
	{
		path.clear();
		player->currentState = (PlayerState*)player->idle;
		if (dustParticles)
		{
			dustParticles->SetActive(false);
		}
	}
}