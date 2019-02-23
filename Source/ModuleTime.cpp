#include "ModuleTime.h"

ModuleTime::ModuleTime()
{
}

ModuleTime::~ModuleTime()
{
}

bool ModuleTime::Init(JSON* json) 
{
	gameDeltaTime = 0.0f;
	frameTimer.Start();
	fpsTimer.Start();
	return true;
}

update_status ModuleTime::Update(float dt) 
{
	++frameCount;
	++realFrameCount;

	// Time update
	realDeltaTime = frameTimer.ReadSeconds();
	realTime += realDeltaTime;

	//App->editor->AddFPSCount(FPS, realDeltaTime * 1000.0f);

	if (gameState == GameState::RUN) 
	{
		//App->editor->AddGameFPSCount(FPS, gameDeltaTime * gameTimeScale * 1000.0f);
		++totalFrames;
		gameDeltaTime = frameTimer.ReadSeconds();
		gameTime += gameDeltaTime * gameTimeScale;
	}
	frameTimer.Reset();

	// Frames per second
	if (fpsTimer.ReadSeconds() >= 1.0f) 
	{
		FPS = frameCount;
		frameCount = 0u;
		fpsTimer.Reset();
	}

	return UPDATE_CONTINUE;
}

bool ModuleTime::CleanUp() 
{
	frameTimer.Stop();
	fpsTimer.Stop();
	return true;
}

void ModuleTime::StartGameClock() 
{
	gameState = GameState::RUN;
}

void ModuleTime::PauseGameClock(bool pause) 
{
	if (pause) 
	{
		gameState = GameState::PAUSE;
		gameDeltaTime = 0.0f;
	}
	else 
	{
		gameState = GameState::RUN;
	}
}

void ModuleTime::StopGameClock() 
{
	gameState = GameState::STOP;
	gameDeltaTime = 0.0f;
	gameTime = 0.0f;
	totalFrames = 0u;
}

void ModuleTime::Step() 
{
	nextFrame = true;
}