#ifndef __LOOPSTATEMENU_H_
#define __LOOPSTATEMENU_H_

#include "LoopState.h"

class Button;
class ComponentImage;

class LoopStateMenu :
	public LoopState
{
public:
	LoopStateMenu(GameLoop* GL);
	virtual ~LoopStateMenu();

	void Update() override;

private:
	void StartGame();

private:
	ComponentImage* introVideo = nullptr;
};

#endif // __LOOPSTATEMENU_H_