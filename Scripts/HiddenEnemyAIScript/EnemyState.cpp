#include "EnemyState.h"

#include "Application.h"
#include "ModuleTime.h"

#include "HiddenEnemyAIScript.h"

EnemyState::EnemyState()
{
}


EnemyState::~EnemyState()
{
}

void EnemyState::UpdateTimer()
{
	timer += enemy->App->time->gameDeltaTime;
}