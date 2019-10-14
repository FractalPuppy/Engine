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
	videoDuration = gLoop->creditsVideo->PlayVideo();
	gLoop->menu->SetActive(false);
	started = true;
}

void LoopStateCredits::Update()
{
	if (!started)
	{
		Enter();
	}
	if (gLoop->creditsVideo != nullptr && gLoop->creditsVideo->videoPlaying)
	{
		videoTimer += gLoop->App->time->gameDeltaTime;
		if (videoTimer >= videoDuration || backButton->IsPressed())
		{
			gLoop->creditsGO->SetActive(false);
			gLoop->creditsVideoGO->SetActive(false);
			gLoop->menu->SetActive(true);
			videoTimer = 0.f;
			gLoop->menuMusic->GetComponent<ComponentAudioSource>()->Play();
			gLoop->creditsAudio->GetComponent<ComponentAudioSource>()->Stop();
			gLoop->currentLoopState = (LoopState*)gLoop->menuState;
			started = false;
		}
	}
}
