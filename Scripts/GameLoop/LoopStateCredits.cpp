#include "LoopStateCredits.h"

#include "GameLoop.h"

#include "GameObject.h"
#include "ComponentButton.h"

#include "CreditsScript.h"

LoopStateCredits::LoopStateCredits(GameLoop* GL) : LoopState(GL)
{
}


LoopStateCredits::~LoopStateCredits()
{
}

void LoopStateCredits::Enter()
{
	gLoop->menu->SetActive(false);
}

void LoopStateCredits::Update()
{
	gLoop->menu->SetActive(false);
	if (!(gLoop->runningCredits))
	{
		gLoop->creditsGO->SetActive(true);
		gLoop->creditsScript->Enable(true);
		gLoop->creditsScript->Start();
		gLoop->runningCredits = true;
	}
	else if (gLoop->creditsScript->creditsDone)
	{
		gLoop->creditsGO->SetActive(false);
		gLoop->menu->SetActive(true);
		gLoop->runningCredits = false;
		gLoop->creditsScript->ResetScript();
		gLoop->currentLoopState = (LoopState*)gLoop->menuState;
	}

	if (gLoop->backCreditsButton->IsPressed())
	{
		gLoop->creditsScript->creditsDone = true;
	}
}
