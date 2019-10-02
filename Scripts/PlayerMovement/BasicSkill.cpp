#include "BasicSkill.h"

#include "Application.h"
#include "ModuleTime.h"

#include "ComponentAnimation.h"
#include "ResourceStateMachine.h"

#include "PlayerState.h"
#include "PlayerMovement.h"
#include "PlayerStateAttack.h"

#include <assert.h>
#include "JSON.h"

BasicSkill::BasicSkill(PlayerMovement* PM, const char* trigger) :
	player(PM), animTrigger(trigger)
{
}

BasicSkill::~BasicSkill()
{
}

void BasicSkill::Start()
{
	if (player == nullptr)
	{
		LOG("Warning: Player GO pointer missing");
	}
}

void BasicSkill::Update()
{
	timer += player->App->time->gameDeltaTime;

	CheckInput();

	if (timer < duration)
	{
		UseSkill();
	}
	else
	{
		Reset();
	}
}

void BasicSkill::Exit()
{
	player->currentState = (PlayerState*)player->idle;
}

void BasicSkill::Expose(ImGuiContext* context)
{
	ImGui::SetCurrentContext(context);

}

void BasicSkill::Serialize(JSON_value* json) const
{
	assert(json != nullptr);

}

void BasicSkill::DeSerialize(JSON_value* json)
{
	assert(json != nullptr);

}

void BasicSkill::Reset()
{
	timer = 0.0f;

	player->currentSkill->Exit();
	player->currentSkill = nullptr;
}