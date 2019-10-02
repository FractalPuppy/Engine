#ifndef __ENEMYSTATE_H_
#define __ENEMYSTATE_H_

class BuriedEnemyAIScript;

#include <vector>
#include "Math/float3.h"

class EnemyState
{
public:
	EnemyState();
	virtual ~EnemyState();

	virtual void HandleIA() {};
	virtual void Update() {};
	virtual void Enter() {};
	virtual void Exit() {};

	void UpdateTimer();
	void ResetTimer() { timer = 0.f; };

public:
	float timer = 0.f;
	float auxTimer = 0.0f;
	float duration = 0.0f;
	std::string trigger;

	BuriedEnemyAIScript* enemy = nullptr;

	// Nav mesh
	std::vector<math::float3> enemyPath;
	float refreshTime = 0.0f;
};

#endif // __ENEMYSTATE_H_