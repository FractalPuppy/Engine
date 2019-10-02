#include "LoopStateDead.h"

#include "GameLoop.h"

#include "ModuleScene.h"

#include "GameObject.h"
#include "ComponentButton.h"


LoopStateDead::LoopStateDead(GameLoop* GL) : LoopState(GL)
{
}


LoopStateDead::~LoopStateDead()
{
}

void LoopStateDead::Update()
{
	if (gLoop->toTheAltarButton->IsPressed())
	{
		gLoop->loseWindow->SetActive(false);
		gLoop->loadingGO->SetActive(true);
		gLoop->playerMenuGO->SetActive(false);
		gLoop->currentLoopState = (LoopState*)gLoop->loadingState;
		if (gLoop->gameScene == GameScene::CEMENTERY)
		{
			gLoop->sceneToLoad = GRAVEYARD_SCENE;
		}
		else if (gLoop->gameScene == GameScene::TEMPLE)
		{
			gLoop->sceneToLoad = TEMPLE_SCENE;
		}
		gLoop->App->scene->stateAfterLoad = "Intro";
		gLoop->stateAfterLoad = (LoopState*)gLoop->introState;
		gLoop->actionAfterLoad = true;
	}
}
