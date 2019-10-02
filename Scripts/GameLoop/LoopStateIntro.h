#ifndef __LOOPSTATEINTRO_
#define __LOOPSTATEINTRO_

#include "LoopState.h"

class LoopStateIntro : public LoopState
{
public:
	LoopStateIntro(GameLoop* GL);
	virtual ~LoopStateIntro();

	void Update() override;
};

#endif // __LOOPSTATEINTRO_