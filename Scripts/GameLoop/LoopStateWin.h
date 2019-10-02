#ifndef __LOOPSTATEWIN_H_
#define __LOOPSTATEWIN_H_

#include "LoopState.h"

class ComponentImage;

class LoopStateWin : public LoopState
{
public:
	LoopStateWin(GameLoop* GL);
	virtual ~LoopStateWin();

	void Update() override;

	void StopLvlMusic();

private:
	ComponentImage* outroVideo = nullptr;
	float outroDuration = 0.0f;
	float outroTimer = 0.0f;
};

#endif // __LOOPSTATEWIN_H_