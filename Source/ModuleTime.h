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
	update_status Update(float dt) override;
	bool CleanUp() override;

	void StartGameClock();
	void PauseGameClock(bool pause);
	void StopGameClock();
	void Step();


private:
	unsigned frameCount = 0u;

public:
	int FPS = 0;
	bool nextFrame = false;
	unsigned maxFps = 144u;
	unsigned totalFrames = 0u;
	GameState gameState = GameState::STOP;

	Timer frameTimer;
	Timer fpsTimer;

	// Game Clock
	float gameTimeScale = 1.0f;
	float gameTime = 0.0f;
	float gameDeltaTime = 0.0f;

	// RealTime Clock
	float realTime = 0.0f;
	float realDeltaTime = 0.0f;
	unsigned realFrameCount = 0u;
};

#endif // __ModuleTime_H__