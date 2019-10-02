#ifndef __ENEMYSTATE_H_
#define __ENEMYSTATE_H_

#include "BaseScript.h"
#include "Math/float3.h"

#include <vector>

#ifdef BasicEnemyAIScript_EXPORTS
#ifndef BasicEnemyAIScript_API
#define BasicEnemyAIScript_API __declspec(dllexport)
#endif // !BasicEnemyAIScript_API
#else
#define BasicEnemyAIScript_API __declspec(dllimport)
#endif

class BasicEnemyAIScript;

class EnemyState
{
public:
	BasicEnemyAIScript_API EnemyState();
	BasicEnemyAIScript_API ~EnemyState();

	virtual void Enter() {};
	BasicEnemyAIScript_API virtual void HandleIA() {};
	virtual void Update() {};
	virtual void Exit() {};
	void UpdateTimer();
	void ResetTimer() { timer = 0.f; };

public:
	float timer = 0.f;
	float auxTimer = 0.0f;
	float duration = 0.0f;
	std::string trigger;

	BasicEnemyAIScript* enemy = nullptr;

protected:
	math::float3 boxSize = math::float3(500.f, 500.f, 500.f);
	math::float3 boxPosition = math::float3::zero;
	float minTime = 0.4f;
	float maxTime = 0.8f;
	bool hitboxCreated = false;

	// Nav mesh
	std::vector<math::float3> enemyPath;
	float refreshTime = 0.0f;
};

#endif // __ENEMYSTATE_H_