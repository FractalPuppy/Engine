#ifndef __PLAYERSTATE_H_
#define __PLAYERSTATE_H_

#include "BaseScript.h"
#include "PlayerMovement.h"

class PlayerState
{
public:
	PlayerState();
	~PlayerState();

	virtual void Update() {};
	virtual void CheckInput() {};
	void UpdateTimer();
	void ResetTimer() { timer = 0.f; };

public:
	bool enabled = false;
	PlayerMovement* player = nullptr;
	std::string trigger;
	float timer = 0.f;
};

#endif // _PLAYERSTATE_H_
