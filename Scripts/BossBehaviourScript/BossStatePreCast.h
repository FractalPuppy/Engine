#ifndef __BOSSSTATEPRECAST_H_
#define __BOSSSTATEPRECAST_H_

#include "BossState.h"

class BossStatePreCast :
	public BossState
{
public:
	BossStatePreCast(BossBehaviourScript* AIBoss);
	~BossStatePreCast();

private:
	void HandleIA() override;
	void Update() override;

	void Enter() override;
	void Exit() override;

	float timerOrbs = 0.0f;
	bool orbsSet = false;
};

#endif // __BOSSSTATEPRECAST_H_