#include "EnemyState.h"

#include "FlyingSkullEnemyAIScript.h"

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