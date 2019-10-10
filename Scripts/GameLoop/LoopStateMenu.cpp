#include "LoopStateMenu.h"
#include "Application.h"
#include "ModuleTime.h"
#include "ModuleInput.h"
#include "GameLoop.h"

#include "GameObject.h"
#include "ComponentButton.h"
#include "ComponentImage.h"

#include "PlayerPrefs.h"

#define GRAVEYARD_SCENE "Level0-TheGraveyard"
#define BUTTONS_NUMBER 4

LoopStateMenu::LoopStateMenu(GameLoop* GL) : LoopState(GL)
{
}


LoopStateMenu::~LoopStateMenu()
{
}

void LoopStateMenu::Update()
{
	for (int i = 0; i < gLoop->menuButtons.size(); ++i)
	{
		if (((Button*)gLoop->menuButtons[i])->IsHovered())
		{
			gLoop->sunHoverGO[BUTTONS_NUMBER - i]->SetActive(true);
		}
		else 
		{
			gLoop->sunHoverGO[BUTTONS_NUMBER - i]->SetActive(false);
		}
	}
	
	if (introVideo != nullptr && introVideo->videoPlaying)
	{
		videoTimer += gLoop->App->time->gameDeltaTime;

		if (gLoop->App->input->AnyKeyPressed())
		{
			if (gLoop->introSkipTextGO->isActive())
			{
				StartGame();
			}
			else
			{
				gLoop->introSkipTextGO->SetActive(true);
			}
		}

		if (videoTimer >= videoDuration)
		{
			StartGame();
		}
	}

	if (((Button*)(gLoop->menuButtons[0]))->IsPressed()) //PlayButton
	{
		if (gLoop->introVideoGO != nullptr && introVideo == nullptr)
		{
			gLoop->introvideoPlaying = true;
			gLoop->introVideoGO->SetActive(true);
			introVideo = gLoop->introVideoGO->GetComponent<ComponentImage>();
			videoDuration = introVideo->PlayVideo();
			gLoop->menu->SetActive(false);
		}
		else if(gLoop->introVideoGO == nullptr)
		{
			StartGame();
		}
	}
	else if (gLoop->optionButton->IsPressed())
	{
		gLoop->options->SetActive(true);
		gLoop->EnableMenuButtons(false);
		gLoop->currentLoopState = (LoopState*)gLoop->optionsState;
	}
	else if (gLoop->controlsButton->IsPressed())
	{
		gLoop->controls->SetActive(true);
		gLoop->EnableMenuButtons(false);
		gLoop->currentLoopState = (LoopState*)gLoop->controlsState;
	}
	else if (gLoop->creditsButton->IsPressed())
	{
		gLoop->creditsVideo = gLoop->creditsVideoGO->GetComponent<ComponentImage>();
		gLoop->currentLoopState = (LoopState*)gLoop->creditsState;
	}
	else if (gLoop->exitButton->IsPressed())
	{
		gLoop->currentLoopState = (LoopState*)gLoop->quitState;
	}
}

void LoopStateMenu::StartGame()
{
	gLoop->currentLoopState = (LoopState*)gLoop->loadingState;
	gLoop->menu->SetActive(false);
	gLoop->loadingGO->SetActive(true);
	if (gLoop->introVideoGO != nullptr)
	{
		gLoop->introVideoGO->SetActive(false);
	}
	gLoop->sceneToLoad = GRAVEYARD_SCENE;
	gLoop->gameScene = GameScene::CEMENTERY;
	PlayerPrefs::DeleteAll(true);
}
