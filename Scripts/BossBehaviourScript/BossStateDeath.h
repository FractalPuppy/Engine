#ifndef __BOSSSTATEDEATH_H_
#define __BOSSSTATEDEATH_H_

#include "BossState.h"

class BossStateDeath :
	public BossState
{
public:
	BossStateDeath(BossBehaviourScript* AIBoss);
	~BossStateDeath();

	void Update() override;
	void Enter() override;

};

#endif // __BOSSSTATEDEATH_H_