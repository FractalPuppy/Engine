#include "LoopStateOptions.h"

#include "GameLoop.h"

#include "GameObject.h"
#include "ComponentButton.h"

LoopStateOptions::LoopStateOptions(GameLoop* GL) : LoopState(GL)
{
}


LoopStateOptions::~LoopStateOptions()
{
}

void LoopStateOptions::Update()
{
	gLoop->VolumeManagement();
	gLoop->SoundManagement();
	gLoop->VsyncManagement();
	gLoop->ResolutionManagement();

	if (gLoop->backOptionButton->IsPressed())
	{
		gLoop->backOptionButton->isHovered = false;
		//gLoop->backOptionButton->isKeyDown = false;
		gLoop->options->SetActive(false);
		gLoop->EnableMenuButtons(true);
		gLoop->currentLoopState = (LoopState*)gLoop->menuState;
	}
}
