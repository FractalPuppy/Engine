#ifndef __LOOPSTATEQUIT_H_
#define __LOOPSTATEQUIT_H_

#include "LoopState.h"

class LoopStateQuit :
	public LoopState
{
public:
	LoopStateQuit(GameLoop* GL);
	virtual ~LoopStateQuit();

	void Update() override;
};

#endif // __LOOPSTATEQUIT_H_