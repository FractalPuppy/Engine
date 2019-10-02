#ifndef __LOOPSTATELOADING_H_
#define __LOOPSTATELOADING_H_

#include "LoopState.h"

class LoopStateLoading :
	public LoopState
{
public:
	LoopStateLoading(GameLoop* GL);
	virtual ~LoopStateLoading();

	void Update() override;
};

#endif // __LOOPSTATELOADING_H_