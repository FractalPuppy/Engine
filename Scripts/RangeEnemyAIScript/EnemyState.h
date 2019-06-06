#ifndef __ENEMYSTATE_H_
#define __ENEMYSTATE_H_

#include "BaseScript.h"
#include "Math/float3.h"
#include <vector>

class RangeEnemyAIScript;

class EnemyState
{
public:
	EnemyState();
	~EnemyState();

	virtual void Enter() {};
	virtual void Update() {};
	virtual void Exit() {};
	void UpdateTimer();
	void ResetTimer() { timer = 0.f; };

public:
	float timer = 0.f;
	float auxTimer = 0.0f;
	std::string trigger;

	RangeEnemyAIScript* enemy = nullptr;

protected:
	bool projectileShooted = false;

	// Nav mesh
	std::vector<math::float3> enemyPath;
	float refreshTime = 0.0f;
};

#endif // __ENEMYSTATE_H_