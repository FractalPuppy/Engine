#ifndef __LOOPSTATECONTROLS_H_
#define __LOOPSTATECONTROLS_H_

#include "LoopState.h"

class GameLoop;
class Button;

class LoopStateControls :
	public LoopState
{
public:
	LoopStateControls(GameLoop* GL);
	virtual ~LoopStateControls();

	void Update() override;
};

#endif // __LOOPSTATECONTROLS_H_