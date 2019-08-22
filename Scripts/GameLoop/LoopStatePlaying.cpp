#include "LoopStatePlaying.h"

#include "GameLoop.h"

#include "GameObject.h"
#include "ComponentButton.h"
#include "ComponentTransform2D.h"

#include "ModuleScene.h"
#include "ModuleTime.h"
#include "ModuleInput.h"
#include "Math/float2.h"
#include "PlayerMovement.h"

#include "ComponentAudioSource.h"
#include "InventoryScript.h"
#include "EquipPopupController.h"
#include "SkillTreeController.h"
#include "ExperienceController.h"
#include "PlayerMovement.h"

LoopStatePlaying::LoopStatePlaying(GameLoop* GL) : LoopState(GL)
{
}


LoopStatePlaying::~LoopStatePlaying()
{
}

void LoopStatePlaying::Update()
{
	if (gLoop->hudBackToMenuButton->IsPressed() ||
	   (!gLoop->playerMenuGO->isActive() && gLoop->App->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN))
	{
		gLoop->currentLoopState = (LoopState*)(gLoop->pausedState);
	}
	if (gLoop->App->input->GetKey(SDL_SCANCODE_P) == KEY_DOWN)
	{
		if (gLoop->playerMenuGO->isActive()) CloseMenu();
		gLoop->currentLoopState = (LoopState*)(gLoop->pausedState);
	}

	if (gLoop->closePlayerMenuButton->KeyUp()) CloseMenu();
	if (gLoop->playerMenuGO->isActive() && gLoop->App->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN) CloseMenu();

	if (gLoop->inventoryButton->IsPressed() || gLoop->App->input->GetKey(SDL_SCANCODE_B) == KEY_DOWN)
	{
		if (gLoop->inventoryMenuGO->isActive()) CloseMenu();
		else OpenMenu(gLoop->inventoryMenuGO);
	}

	if (gLoop->skillsButton->IsPressed() || gLoop->App->input->GetKey(SDL_SCANCODE_V) == KEY_DOWN)
	{
		if (gLoop->skillsMenuGO->isActive()) CloseMenu();
		else OpenMenu(gLoop->skillsMenuGO);
	}
	if (gLoop->inventoryButton->IsPressed() && !gLoop->inventoryMenuGO->isActive())	gLoop->menuButtonsSound->Play();
	if (gLoop->skillsButton->IsPressed() && !gLoop->skillsMenuGO->isActive()) gLoop->menuButtonsSound->Play();
	if (gLoop->missionsButton->IsPressed() && !gLoop->missionsMenuGO->isActive()) gLoop->menuButtonsSound->Play();

	if (gLoop->missionsButton->IsPressed() || gLoop->App->input->GetKey(SDL_SCANCODE_N) == KEY_DOWN)
	{
		if (gLoop->missionsMenuGO->isActive()) CloseMenu();
		else OpenMenu(gLoop->missionsMenuGO);
	}
	
	if (gLoop->playerScript->isPlayerDead)
	{
		gLoop->loseWindow->SetActive(true);
		gLoop->currentLoopState = (LoopState*)gLoop->deadState;
	}
	else if (gLoop->winBbox->Intersects(*(gLoop->playerBbox)))
	{

		if (gLoop->gameScene == GameScene::CEMENTERY || gLoop->gameScene == GameScene::TEMPLE)
		{
			gLoop->inventoryScript->SaveInventory();
			gLoop->skillTreeScript->SaveSkillTree();
			gLoop->experienceScript->SaveExperience();
			gLoop->equipPopUpScript->SavePopUp();
			gLoop->playerScript->SavePlayerStats();
		}
		

		if (gLoop->gameScene == GameScene::CEMENTERY)
		{
			gLoop->currentLoopState = (LoopState*)gLoop->loadingState;
			gLoop->loadingGO->SetActive(true);
			gLoop->playerMenuGO->SetActive(false);
			gLoop->sceneToLoad = TEMPLE_SCENE;
			gLoop->App->scene->actionAfterLoad = true;
			gLoop->App->scene->stateAfterLoad = "Temple";
			gLoop->stateAfterLoad = (LoopState*)gLoop->creditsState;
			gLoop->gameScene = GameScene::TEMPLE;
			
		}
		else if (gLoop->gameScene == GameScene::TEMPLE)
		{
			gLoop->winWindow->SetActive(true);
			gLoop->currentLoopState = (LoopState*)gLoop->winState;
			gLoop->gameScene = GameScene::MENU;
		}
	}
}

void LoopStatePlaying::LoadMainMenu()
{
	gLoop->currentLoopState = (LoopState*)gLoop->loadingState;
	gLoop->playerMenuGO->SetActive(false);
	gLoop->hudGO->SetActive(false);
	gLoop->loadingGO->SetActive(true);
	gLoop->sceneToLoad = MENU_SCENE;
}

void LoopStatePlaying::OpenMenu(GameObject * menu)
{
	gLoop->playerMenuGO->SetActive(true);
	
	gLoop->inventoryButton->rectTransform->setPosition(math::float2(-485, gLoop->inventoryButton->rectTransform->getPosition().y));
	gLoop->skillsButton->rectTransform->setPosition(math::float2(-485, gLoop->skillsButton->rectTransform->getPosition().y));
	gLoop->missionsButton->rectTransform->setPosition(math::float2(-485, gLoop->missionsButton->rectTransform->getPosition().y));
	
	gLoop->inventoryMenuGO->SetActive(menu == gLoop->inventoryMenuGO);
	gLoop->skillsMenuGO->SetActive(   menu == gLoop->skillsMenuGO);
	gLoop->missionsMenuGO->SetActive( menu == gLoop->missionsMenuGO);
}

void LoopStatePlaying::CloseMenu()
{
	gLoop->closePlayerMenuButton->isHovered = false;

	gLoop->inventoryMenuGO->SetActive(false);
	gLoop->skillsMenuGO->SetActive(false);
	gLoop->missionsMenuGO->SetActive(false);
	gLoop->playerMenuGO->SetActive(false);

	gLoop->inventoryButton->rectTransform->setPosition(math::float2(-50, gLoop->inventoryButton->rectTransform->getPosition().y));
	gLoop->skillsButton->rectTransform->setPosition(math::float2(-50, gLoop->skillsButton->rectTransform->getPosition().y));
	gLoop->missionsButton->rectTransform->setPosition(math::float2(-50, gLoop->missionsButton->rectTransform->getPosition().y));
}
