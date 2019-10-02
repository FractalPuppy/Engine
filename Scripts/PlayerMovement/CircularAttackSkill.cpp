#include "CircularAttackSkill.h"

#include "PlayerMovement.h"

#include "ModuleTime.h"
#include "ModuleInput.h"
#include "ModuleNavigation.h"
#include "ModuleScene.h"

#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentBoxTrigger.h"

#include "PlayerState.h"


#include "Math/float2.h"

CircularAttackSkill::CircularAttackSkill(PlayerMovement* PM, const char* trigger, ComponentBoxTrigger* attackBox) :
	MeleeSkill(PM, trigger, attackBox)
{
}

CircularAttackSkill::~CircularAttackSkill()
{
}

void CircularAttackSkill::Start()
{
	fullSpinTime = duration;

	MeleeSkill::Start();

	player->gameobject->transform->LookAtMouse();

	// Create the hitbox
	boxSize = math::float3(250.f, 250.f, 250.f);

	// Set delay for hit
	hitDelay = 0.3f;

}

void CircularAttackSkill::Update()
{
	//BasicSkill::Update() modified

	timer += player->App->time->gameDeltaTime;

	if (timer < fullSpinTime * (numSpins + 1u))
	{
		UseSkill();
	}
	else
	{
		Reset();
		return;
	}

	CheckInput();

	// MeleeSkill::Update() modified. Check when it is time to enable the hitbox
	if (!atatckStarted && !attackBoxTrigger->enabled && timer > hitDelay)
	{
		spinTimer = 0.0f;
		attackBoxTrigger->Enable(true);
		attackBoxTrigger->SetBoxSize(boxSize);
		atatckStarted = true;

		if (mesh3)
		{
			mesh3->SetActive(true);
		}
		if (particles)
		{
			particles->SetActive(true);
		}
		if (player->slashTrail)
		{
			player->slashTrail->SetActive(true);
		}
	}
}

void CircularAttackSkill::UseSkill()
{
	if (player->attackBoxTrigger != nullptr && !player->attackBoxTrigger->enabled && timer < player->currentState->duration * (numSpins + 1u))
	{
		// Update hitbox
		player->attackBoxTrigger->SetBoxPosition(boxPosition.x, boxPosition.y, boxPosition.z);
	}

	// Check spin state
	if (player->attackBoxTrigger != nullptr && player->attackBoxTrigger->enabled)
	{
		// Full spin performed?
		if (spinTimer > fullSpinTime)
		{
			attackBoxTrigger->Enable(false);
		}
		else
		{
			spinTimer += player->App->time->gameDeltaTime;
		}
	}
	else if(atatckStarted)
	{
		// Go for next spin
		spinTimer = 0.0f;
		attackBoxTrigger->Enable(true);
	}

	//Rotate meshes
	
	if (mesh3)
	{
		mesh3->transform->Rotate(math::float3(0, MESH3_ROT_SPEED, 0));
	}
}

void CircularAttackSkill::Reset()
{
	spinTimer = 0.0f;
	atatckStarted = false;
	MeleeSkill::Reset();

	if (mesh3)
	{
		mesh3->SetActive(false);
	}
	if (particles)
	{
		particles->SetActive(false);
	}

	if (player->slashTrail)
	{
		player->slashTrail->SetActive(false);
	}
}

void CircularAttackSkill::CheckInput()
{
	// Once the attack is finished
	if (timer > fullSpinTime * (numSpins + 1u))
	{
		if (player->IsUsingSkill())
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
	}
	else
	{
		// Move while using the attack
		MoveSpinning();
	}
}

void CircularAttackSkill::MoveSpinning()
{
	if (player->IsMoving())
	{
	math:float2 mouse((float*)&player->App->input->GetMousePosition());
		if (player->App->input->GetMouseButtonDown(3) == KEY_DOWN
			|| player->App->input->GetMouseButtonDown(3) == KEY_REPEAT)
		{
			moveTimer = 0.0f;
			math::float3 intPos(0.f, 0.f, 0.f);
			if (player->App->navigation->NavigateTowardsCursor(player->gameobject->transform->position, path,
				math::float3(player->OutOfMeshCorrectionXZ, player->OutOfMeshCorrectionY, player->OutOfMeshCorrectionXZ),
				intPos, player->maxWalkingDistance))
			{
				//case the player clicks outside of the floor mesh but we want to get close to the floors edge
				pathIndex = 0;
			}
			else
			{
				return;
			}
		}
		else if (player->App->input->GetMouseButtonDown(3) == KEY_REPEAT)
		{
			moveTimer += player->App->time->gameDeltaTime;
		}
		if (path.size() > 0)
		{
			math::float3 currentPosition = player->gameobject->transform->GetPosition();
			while (pathIndex < path.size() && currentPosition.DistanceSq(path[pathIndex]) < MINIMUM_PATH_DISTANCE)
			{
				pathIndex++;
			}
			if (pathIndex < path.size())
			{
				player->gameobject->transform->LookAt(path[pathIndex]);
				math::float3 direction = (path[pathIndex] - currentPosition).Normalized();
				player->gameobject->transform->SetPosition(currentPosition + player->walkingSpeed * direction * player->App->time->gameDeltaTime);
			}
		}
	}
}

