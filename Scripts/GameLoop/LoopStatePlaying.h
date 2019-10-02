#ifndef __LOOPSTATEPLAYING_H_
#define __LOOPSTATEPLAYING_H_

#include "LoopState.h"

class Button;

class LoopStatePlaying :
	public LoopState
{
public:
	LoopStatePlaying(GameLoop* GL);
	virtual ~LoopStatePlaying();
	
	void Update() override;


private:
	void Win();
	void LoadMainMenu();
	void OpenMenu(GameObject* menu);
	void CloseMenu();
};

#endif // __LOOPSTATEPLAYING_H_