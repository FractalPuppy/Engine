#ifndef __PLAYERSTATE_H_
#define __PLAYERSTATE_H_

#include "BaseScript.h"
#include "Math/float3.h"
class PlayerMovement;

class PlayerState
{
public:
	PlayerState(PlayerMovement * PM, const char* trigger,
		math::float3 boxSize = math::float3(500.f, 500.f, 500.f), 
		float minTime = 0.4f, float maxTime = 0.8f);

	virtual ~PlayerState();

	virtual void Enter() {};
	virtual void Update() {};
	virtual void Exit() {};
	virtual void CheckInput() {};
	void UpdateTimer();
	void ResetTimer() { timer = 0.f; };

public:
	bool enabled = false;
	float duration = 0.f;
	PlayerMovement* player = nullptr;
	std::string trigger;
	float timer = 0.f;
	bool playerWalking = false;

protected:
	math::float3 boxSize = math::float3::zero;
	math::float3 boxPosition = math::float3::zero;
	float minTime = 0.f;
	float maxTime = 0.f;

	bool hitboxCreated = false;
};

#endif // _PLAYERSTATE_H_
