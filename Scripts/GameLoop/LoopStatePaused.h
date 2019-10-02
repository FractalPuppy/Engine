#ifndef __LOOPSTATEPAUSED_H_
#define __LOOPSTATEPAUSED_H_

#include "LoopState.h"

class LoopStatePaused :
	public LoopState
{
public:
	LoopStatePaused(GameLoop* GL);
	virtual ~LoopStatePaused();

	void Enter() override;
	void Update() override;
	void Exit() override;
};

#endif // __LOOPSTATEPAUSED_H_