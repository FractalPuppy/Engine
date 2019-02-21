#include "Timer.h"
#include "SDL_timer.h"


Timer::Timer()
{
}

Timer::~Timer()
{
}

void Timer::Start()
{
	initTime = SDL_GetTicks();
	timerOn = true;
}

int Timer::Read()
{
	float time = 0.0f;

	if (timerOn)
	{
		time = (SDL_GetTicks() - initTime + skippedTime);
	}

	return time;
}

float Timer::ReadSeconds()
{
	float time = 0.0f;

	if (timerOn)
	{
		time = (SDL_GetTicks() - initTime + skippedTime) / 1000.0f;
	}

	return time;
}

void Timer::Pause()
{
	skippedTime += (SDL_GetTicks() - initTime);
	timerOn = false;
}

int Timer::Stop()
{
	timerOn = false;
	totalTime = SDL_GetTicks() - initTime;
	return totalTime;
}

void Timer::Reset()
{
	initTime = SDL_GetTicks();
	skippedTime = 0;
}
