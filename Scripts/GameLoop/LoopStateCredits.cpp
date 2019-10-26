#include "LoopStateCredits.h"

#include "GameLoop.h"

#include "Application.h"
#include "ModuleTime.h"
#include "ModuleScene.h"

#include "GameObject.h"
#include "ComponentButton.h"
#include "ComponentImage.h"
#include "ComponentAudioSource.h"

#include "CreditsScript.h"

LoopStateCredits::LoopStateCredits(GameLoop* GL) : LoopState(GL)
{
}


LoopStateCredits::~LoopStateCredits()
{
}

void LoopStateCredits::Enter()
{
	
	backButtonGO = gLoop->App->scene->FindGameObjectByName("Back", gLoop->App->scene->canvas);
	backButton = backButtonGO->GetComponent<Button>();
	if (gLoop->creditsVideoGO != nullptr && gLoop->creditsVideo == nullptr)
	{
		gLoop->creditsVideo = gLoop->introVideoGO->GetComponent<ComponentImage>();
	}
	else if (gLoop->creditsVideoGO == nullptr)
	{
		gLoop->currentLoopState = (LoopState*)gLoop->menuState;
		return;
	}
	if (gLoop->menuMusic != nullptr)
	{
		gLoop->menuMusic->GetComponent<ComponentAudioSource>()->Stop();
	}
	gLoop->creditsAudio->GetComponent<ComponentAudioSource>()->Play();
	gLoop->creditsGO->SetActive(true);
	gLoop->creditsVideoGO->SetActive(true);
	gLoop->creditsVideo->PlayVideo();
	gLoop->menu->SetActive(false);
	started = true;
	gLoop->creditsPlaying = true;
}

void LoopStateCredits::Update()
{
	if (!started)
	{
		Enter();
	}
	if (gLoop->creditsVideo != nullptr && gLoop->creditsVideo->videoPlaying)
	{
		if (backButton->IsPressed())
		{
			SetMenu();
		}
	}
	else if (gLoop->creditsVideo->videoFinished && started)
	{
		SetMenu();
	}
}

void LoopStateCredits::SetMenu()
{
	gLoop->creditsPlaying = false;
	gLoop->creditsAudio->GetComponent<ComponentAudioSource>()->Stop();
	gLoop->creditsGO->SetActive(false);
	gLoop->creditsVideoGO->SetActive(false);
	gLoop->menu->SetActive(true);
	if (gLoop->menuMusic != nullptr)
	{
		gLoop->menuMusic->GetComponent<ComponentAudioSource>()->Play();
	}
	gLoop->currentLoopState = (LoopState*)gLoop->menuState;
	gLoop->creditsVideo->videoFinished = false;
	gLoop->creditsVideo->videoPlaying = false;
	started = false;
}
