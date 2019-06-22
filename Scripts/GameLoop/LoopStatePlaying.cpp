#include "LoopStatePlaying.h"

#include "GameLoop.h"

#include "GameObject.h"
#include "ComponentButton.h"
#include "ComponentTransform2D.h"

#include "ModuleInput.h"
#include "Math/float2.h"
#include "PlayerMovement.h"

#define MENU_SCENE "MenuScene"

LoopStatePlaying::LoopStatePlaying(GameLoop* GL) : LoopState(GL)
{
}


LoopStatePlaying::~LoopStatePlaying()
{
}

void LoopStatePlaying::HandleHotkeys()
{
	if (gLoop->App->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
	{
		gLoop->closePlayerMenuButton->isKeyUp = true;
	}
	else if (gLoop->App->input->GetKey(SDL_SCANCODE_B) == KEY_DOWN)
	{
		gLoop->inventoryButton->isKeyDown = true;
	}
	else if (gLoop->App->input->GetKey(SDL_SCANCODE_N) == KEY_DOWN)
	{
		gLoop->missionsButton->isKeyDown = true;
	}
	else if (gLoop->App->input->GetKey(SDL_SCANCODE_V) == KEY_DOWN)
	{
		gLoop->skillsButton->isKeyDown = true;
	}
	else if (gLoop->App->input->GetKey(SDL_SCANCODE_M) == KEY_DOWN)
	{
		//This will open the map
	}
}

void LoopStatePlaying::Update()
{
	HandleHotkeys();

	if (gLoop->hudBackToMenuButton->IsPressed())
	{
		gLoop->currentLoopState = (LoopState*)gLoop->loadingState;
		gLoop->playerMenuGO->SetActive(false);
		gLoop->hudGO->SetActive(false);
		gLoop->loadingGO->SetActive(true);
		gLoop->sceneToLoad = MENU_SCENE;
	}

	if (gLoop->closePlayerMenuButton->KeyUp())
	{
		gLoop->closePlayerMenuButton->isHovered = false;
		gLoop->playerMenuGO->SetActive(false);
		gLoop->inventoryButton->rectTransform->setPosition(math::float2(-50, gLoop->inventoryButton->rectTransform->getPosition().y));
		gLoop->skillsButton->rectTransform->setPosition(math::float2(-50, gLoop->skillsButton->rectTransform->getPosition().y));
		gLoop->missionsButton->rectTransform->setPosition(math::float2(-50, gLoop->missionsButton->rectTransform->getPosition().y));

	}

	if (gLoop->inventoryButton->IsPressed() || gLoop->skillsButton->IsPressed() || gLoop->missionsButton->IsPressed())
	{
		gLoop->playerMenuGO->SetActive(true);
		gLoop->inventoryButton->rectTransform->setPosition(math::float2(-485, gLoop->inventoryButton->rectTransform->getPosition().y));
		gLoop->skillsButton->rectTransform->setPosition(math::float2(-485, gLoop->skillsButton->rectTransform->getPosition().y));
		gLoop->missionsButton->rectTransform->setPosition(math::float2(-485, gLoop->missionsButton->rectTransform->getPosition().y));
		gLoop->inventoryMenuGO->SetActive(gLoop->inventoryButton->IsPressed());
		gLoop->skillsMenuGO->SetActive(gLoop->skillsButton->IsPressed());
		gLoop->missionsMenuGO->SetActive(gLoop->missionsButton->IsPressed());
	}

	if (gLoop->playerScript->isPlayerDead)
	{
		gLoop->loseWindow->SetActive(true);
		gLoop->currentLoopState = (LoopState*)gLoop->deadState;
	}
	else if (gLoop->winBbox->Intersects(*(gLoop->playerBbox)))
	{
		gLoop->winWindow->SetActive(true);
		gLoop->currentLoopState = (LoopState*)gLoop->winState;
	}
}
