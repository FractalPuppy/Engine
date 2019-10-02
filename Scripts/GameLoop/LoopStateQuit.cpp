#include "LoopStateQuit.h"
#include "Application.h"

#include "GameLoop.h"

#include "ModuleScript.h"


LoopStateQuit::LoopStateQuit(GameLoop* GL) : LoopState(GL)
{
}


LoopStateQuit::~LoopStateQuit()
{
}

void LoopStateQuit::Update()
{
	gLoop->App->scripting->status = UPDATE_STOP;
}
