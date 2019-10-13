#ifndef __BOSSSTATEINTERPHASE_H_
#define __BOSSSTATEINTERPHASE_H_

#include "BossState.h"

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

	bool finished = false;
	float powerUpTimer = 0.0f;
	float relocateTimer = 0.0f;
	bool durationPowerSet = false;
};

#endif // __BOSSSTATEINTERPHASE_H_