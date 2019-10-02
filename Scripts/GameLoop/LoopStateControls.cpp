#include "LoopStateControls.h"

#include "GameLoop.h"

#include "GameObject.h"
#include "ComponentButton.h"

LoopStateControls::LoopStateControls(GameLoop* GL) : LoopState(GL)
{
}


LoopStateControls::~LoopStateControls()
{
}

void LoopStateControls::Update()
{
	if (gLoop->backControlsButton->IsPressed())
	{
		gLoop->backControlsButton->isHovered = false;
		//gLoop->backControlsButton->isKeyDown = false;
		gLoop->controls->SetActive(false);
		gLoop->EnableMenuButtons(true);
		gLoop->currentLoopState = (LoopState*)gLoop->menuState;
	}
}
