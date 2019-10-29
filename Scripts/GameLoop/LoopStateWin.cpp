#include "LoopStateWin.h"
#include "Application.h"

#include "ModuleTime.h"
#include "ModuleScene.h"
#include "ModuleInput.h"

#include "ComponentAudioSource.h"
#include "ComponentImage.h"
#include "GameLoop.h"

#include "GameObject.h"

#define MENU_SCENE "MenuScene"

LoopStateWin::LoopStateWin(GameLoop* GL) : LoopState(GL)
{
}


LoopStateWin::~LoopStateWin()
{
}

void LoopStateWin::Update()
{
	//First Outro video then loading for credits
	//TODO Disable UI!!
	if (outroVideo == nullptr && gLoop->outroVideoGO != nullptr)
	{
		gLoop->outroVideoGO->SetActive(true);
		outroVideo = gLoop->outroVideoGO->GetComponent<ComponentImage>();
		outroVideo->PlayVideo();
		StopLvlMusic();
	}

	if (outroVideo != nullptr && !outroVideo->videoFinished)
	{
		if (gLoop->App->input->AnyKeyPressed())
		{
			if (!gLoop->outroSkipTextGO->isActive())
			{
				gLoop->outroSkipTextGO->SetActive(true);
				return;
			}
			else 
			{
				outroVideo->StopVideo();
			}
		}
		else 
		{
			return;
		}
	}


	if (!gLoop->loadingGO->isActive())
	{
		gLoop->loadingGO->SetActive(true);
	}

	gLoop->currentLoopState = (LoopState*)gLoop->loadingState;
	gLoop->playerMenuGO->SetActive(false);
	gLoop->sceneToLoad = MENU_SCENE;
	gLoop->App->scene->actionAfterLoad = true;
	gLoop->App->scene->stateAfterLoad = "Credits";
	gLoop->stateAfterLoad = (LoopState*)gLoop->creditsState;
}

void LoopStateWin::StopLvlMusic() //Temporal Fix to stop all music //TODO: we should have a musicLvlController
{
	if (gLoop->audioGO == nullptr) return;

	for (Component* audioSource : gLoop->audioGO->GetComponentsInChildren(ComponentType::AudioSource))
	{
		((ComponentAudioSource*) audioSource)->Stop();
	}
	gLoop->audioGO->SetActive(false);
}
