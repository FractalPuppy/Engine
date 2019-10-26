#include "LoopStateIntro.h"

#include "Application.h"
#include "GameLoop.h"

#include "GameObject.h"
#include "ComponentButton.h"
#include "ComponentCamera.h"

#include "ModuleScene.h"

LoopStateIntro::LoopStateIntro(GameLoop* GL) : LoopState(GL)
{
}


LoopStateIntro::~LoopStateIntro()
{
}

void LoopStateIntro::Update()
{
	gLoop->hudGO->SetActive(true);
	GameObject* playerCameraGO = gLoop->App->scene->FindGameObjectByName("PlayerCamera");
	ComponentCamera* camera = (ComponentCamera*)playerCameraGO->GetComponent<ComponentCamera>();
	camera->SetAsMain();
	gLoop->currentLoopState = (LoopState*)gLoop->playingState;
}
