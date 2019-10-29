#ifndef __BOSSSTATEINTERPHASE_H_
#define __BOSSSTATEINTERPHASE_H_

#include "BossState.h"

class ComponentRenderer;

enum class IpState
{
	None,
	Fadeoff,
	TP,
	FadeIn,
	Powerup,
	Relocate,
	Finished
};

enum class FX
{
	None,
	Charge,
	Loop,
	Explode,
	Finished
};

class BossStateInterPhase :
	public BossState
{
public:
	BossStateInterPhase(BossBehaviourScript* AIBoss);
	~BossStateInterPhase();

private:

	void HandleIA() override;
	void Update() override;
	void Enter() override;
	void Exit() override;

	IpState ipState = IpState::None;
	FX fxState = FX::None;

	bool finished = false;
	float powerUpTimer = 0.0f;
	float relocateTimer = 0.0f;
	float fxTimer = 0.0f;
	float offset = 0.0f;
	float angle = 0.0f;

	bool durationPowerSet = false;
	bool particlesFX = false;
	bool fxBigBallSet = false;
	bool fxHandBallSet = false;

	ComponentRenderer* ballRenderer = nullptr;
	math::float3 originalScale = math::float3::zero;
};

#endif // __BOSSSTATEINTERPHASE_H_