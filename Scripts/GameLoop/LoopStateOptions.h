#ifndef __LOOPSTATEOPTIONS_H_
#define __LOOPSTATEOPTIONS_H_

#include "LoopState.h"

class LoopStateOptions :
	public LoopState
{
public:
	LoopStateOptions(GameLoop* GL);
	virtual ~LoopStateOptions();

	void Update() override;
};

#endif // __LOOPSTATEOPTIONS_H_