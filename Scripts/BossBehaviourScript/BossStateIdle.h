#ifndef _BOSSSTATEIDLE_H_
#define _BOSSSTATEIDLE_H_

#include "BossState.h"

class BossStateIdle :
	public BossState
{
public:
	BossStateIdle(BossBehaviourScript* AIBoss);
	~BossStateIdle();

	void HandleIA() override;
	void Update() override;

	void Enter() override;

private:
	float baseDuration = 2.0f;

};

#endif // _BOSSSTATEIDLE_H_