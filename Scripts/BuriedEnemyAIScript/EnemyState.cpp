#include "EnemyState.h"

#include "BuriedEnemyAIScript.h"
#include "Application.h"
#include "ModuleTime.h"

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