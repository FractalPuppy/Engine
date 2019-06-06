#ifndef __ModuleTime_H__
#define __ModuleTime_H__

#include "Module.h"

#include "Timer.h"

enum class GameState 
{ 
	RUN,
	PAUSE,
	STOP
};

class ModuleTime :
	public Module
{
public:
	ModuleTime();
	virtual ~ModuleTime();

	bool Init(JSON* json) override;

	void UpdateTime();
	void ResetGameDetaTime();
	bool IteratePartition();
	bool CleanUp() override;

	void StartGameClock();
	void PauseGameClock(bool pause);
	void StopGameClock();
	void Step();

private:
	void PartitionTime();

private:
	unsigned frameCount = 0u;
	float aggregateGameDeltaTime = 0.0f;

public:
	int FPS = 0;
	bool nextFrame = false;
	bool isTimePartitioned = false;
	unsigned maxFps = 144u;
	unsigned totalFrames = 0u;
#ifndef GAME_BUILD
	GameState gameState = GameState::STOP;
#else
	GameState gameState = GameState::RUN;
#endif
	Timer frameTimer;
	Timer fpsTimer;

	// Game Clock
	float gameTimeScale = 1.0f;
	float gameTime = 0.0f;
	float gameDeltaTime = 0.0f;
	float fullGameDeltaTime = 0.0f;

	// RealTime Clock
	float realTime = 0.0f;
	float realDeltaTime = 0.0f;
	unsigned realFrameCount = 0u;
};

#endif // __ModuleTime_H__