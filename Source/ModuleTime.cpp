#include "ModuleTime.h"

#define MAX_FRAME_MS 0.04F

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

void ModuleTime::UpdateTime() 
{
	++frameCount;
	++realFrameCount;

	// Time update
	realDeltaTime = frameTimer.ReadSeconds();
	realTime += realDeltaTime;

	++totalFrames;
	gameDeltaTime = frameTimer.ReadSeconds();
	fullGameDeltaTime = gameDeltaTime;

	gameTime += gameDeltaTime * gameTimeScale;
	
	frameTimer.Reset();

	// Frames per second
	if (fpsTimer.ReadSeconds() >= 1.0f) 
	{
		FPS = frameCount;
		frameCount = 0u;
		fpsTimer.Reset();
	}

	if (gameDeltaTime > MAX_FRAME_MS)
	{
		PartitionTime();
	}
	else
	{
		isTimePartitioned = false;
	}
}

void ModuleTime::ResetGameDetaTime()
{
	gameDeltaTime = 0.0f;
	fullGameDeltaTime = 0.0f;
	isTimePartitioned = false;
}

void ModuleTime::PartitionTime()
{
	gameDeltaTime = MAX_FRAME_MS;
	aggregateGameDeltaTime = gameDeltaTime;

	isTimePartitioned = true;
}

bool ModuleTime::IteratePartition()
{
	if (aggregateGameDeltaTime < fullGameDeltaTime)
	{
		if (aggregateGameDeltaTime + MAX_FRAME_MS <= fullGameDeltaTime)
		{
			gameDeltaTime = MAX_FRAME_MS;
		}
		else
		{
			gameDeltaTime = fullGameDeltaTime - aggregateGameDeltaTime;
		}
		aggregateGameDeltaTime += MAX_FRAME_MS;
		return true;
	}
	isTimePartitioned = false;
	return false;
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