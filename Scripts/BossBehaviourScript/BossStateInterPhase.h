#ifndef __BOSSSTATEINTERPHASE_H_
#define __BOSSSTATEINTERPHASE_H_

#include "BossState.h"
class BossStateInterPhase :
	public BossState
{
public:
	BossStateInterPhase(BossBehaviourScript* AIBoss);
	~BossStateInterPhase();
};

#endif // __BOSSSTATEINTERPHASE_H_