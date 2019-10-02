#include "Application.h"
#include "ModuleTime.h"

#include "BossState.h"

#include "BossBehaviourScript.h"


BossState::BossState()
{
}


BossState::~BossState()
{
}

void BossState::UpdateTimer()
{
	timer += boss->App->time->gameDeltaTime;
}
