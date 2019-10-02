#ifndef __LOOPSTATEDEAD_H_
#define __LOOPSTATEDEAD_H_

#include "LoopState.h"

class LoopStateDead :
	public LoopState
{
public:
	LoopStateDead(GameLoop* GL);
	virtual ~LoopStateDead();

	void Update() override;
};

#endif // __LOOPSTATEDEAD_H_