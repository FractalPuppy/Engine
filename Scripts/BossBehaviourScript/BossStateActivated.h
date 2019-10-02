#ifndef _BOSSSTATEACTIVATED_H_
#define _BOSSSTATEACTIVATED_H_

#include "BossState.h"
#include "Math/float3.h"

class BossStateActivated :
	public BossState
{
public:
	BossStateActivated(BossBehaviourScript* AIBoss);
	~BossStateActivated();

private:
	void HandleIA() override;
	void Update() override;
	void Enter() override;

	float speedUntilHigh = 0.0f;
	float speedDescend = 0.0f;

	math::float3 directionToHigh = math::float3::zero;
	math::float3 directionToGround = math::float3::zero;

	bool cameraShaking = false;
	float percToHigh = 0.3f;
	float percFloating = 0.4f;
	float timeToHighPoint = 0.0f;
	float timeFloating = 0.0f;
	float timeToGround = 0.0f;
};

#endif // _BOSSSTATEACTIVATED_H_