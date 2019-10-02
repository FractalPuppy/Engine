#ifndef __LOOPSTATECREDITS_H_
#define __LOOPSTATECREDITS_H_

#include "LoopState.h"

class CreditsScript;

class LoopStateCredits :
	public LoopState
{
public:
	LoopStateCredits(GameLoop* GL);
	virtual ~LoopStateCredits();

	void Enter() override;
	void Update() override;
};

#endif // __LOOPSTATECREDITS_H_