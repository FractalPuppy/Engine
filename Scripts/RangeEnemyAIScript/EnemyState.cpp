#include "EnemyState.h"

#include "Application.h"
#include "ModuleTime.h"

#include "RangeEnemyAIScript.h"

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