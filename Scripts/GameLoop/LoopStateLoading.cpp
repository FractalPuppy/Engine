#include "LoopStateLoading.h"
#include "Application.h"

#include "GameLoop.h"

#include "ModuleScene.h"


LoopStateLoading::LoopStateLoading(GameLoop* GL) : LoopState(GL)
{
}


LoopStateLoading::~LoopStateLoading()
{
}

void LoopStateLoading::Update()
{
	gLoop->App->scene->loadScene = true;
	gLoop->App->scene->name = gLoop->sceneToLoad;
}
