#ifndef __Timer_h__
#define __Timer_h__

class Timer
{
public:
	Timer();
	~Timer();

	void Start();
	int Read();
	float ReadSeconds();
	void Pause();
	int Stop();
	void Reset();

private:
	int initTime = 0;
	int skippedTime = 0;

public:
	int totalTime = 0;
	bool timerOn = false;
};

#endif	//__Timer_h__

