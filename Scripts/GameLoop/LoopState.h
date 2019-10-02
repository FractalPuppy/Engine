#ifndef __LOOPSTATE_H_
#define __LOOPSTATE_H_

#include "BaseScript.h"

#define MENU_SCENE "MenuScene"
#define GRAVEYARD_SCENE "Level0-TheGraveyard"
#define TEMPLE_SCENE "Level2-ForbiddenTemple"

class GameLoop;

class LoopState
{
public:
	LoopState(GameLoop* GL);
	virtual ~LoopState();

	virtual void Update() {};
	virtual void Enter() {};
	virtual void Exit() {};


public:

	GameLoop* gLoop = nullptr;

};

#endif // __LOOPSTATE_H_