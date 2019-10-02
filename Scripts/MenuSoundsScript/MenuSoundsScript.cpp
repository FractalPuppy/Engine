#include "Application.h"
#include "MenuSoundsScript.h"
#include "ModuleScene.h"
#include "ComponentButton.h"
#include "ComponentAudioSource.h"

#include "soloud.h"
#include "soloud_wav.h"
#include "soloud_wavstream.h"

#include "GameObject.h"

#include "GameLoop.h"

MenuSoundsScript_API Script* CreateScript()
{
	MenuSoundsScript* instance = new MenuSoundsScript;
	return instance;
}

void MenuSoundsScript::Start()
{

	Menu = App->scene->FindGameObjectByName("Menu");
	buttons = Menu->GetComponentsInChildren(ComponentType::Button);

	audioHovered = (ComponentAudioSource*)App->scene->FindGameObjectByName("HoveredAudio")->GetComponentOld(ComponentType::AudioSource);
	assert(audioHovered != nullptr);

	audioClick = (ComponentAudioSource*)App->scene->FindGameObjectByName("ClickAudio")->GetComponentOld(ComponentType::AudioSource);
	assert(audioClick != nullptr);

	ambienceMusic = App->scene->FindGameObjectByName("Ambience")->GetComponent<ComponentAudioSource>();
	assert(ambienceMusic != nullptr);

	mainMenuMusic = (ComponentAudioSource*)App->scene->FindGameObjectByName("Main Music")->GetComponentOld(ComponentType::AudioSource);
	assert(mainMenuMusic != nullptr);

	GameObject* gameController = App->scene->FindGameObjectByName("GameController");
	gameLoopScript = gameController->GetComponent<GameLoop>();
	assert(gameLoopScript != nullptr);

}

void MenuSoundsScript::Update()
{
	bool nothingHovered = true;
	bool nothingClicked = true;

	// Check if button clicked or hovered
	for (auto button : buttons)
	{

		if (((Button*)button)->IsHovered())
		{
			if (!itemHovered) audioHovered->Play();
			itemHovered = true;
			nothingHovered = false;
		}
		if (((Button*)button)->IsPressed())
		{
			if (!itemClicked) audioClick->Play();
			itemClicked = true;
			nothingClicked = false;
		}
	}

	itemHovered = !nothingHovered;
	itemClicked = !nothingClicked;

	if (gameLoopScript->introvideoPlaying)
	{
		mainMenuMusic->SetVolume(0.f);
		ambienceMusic->SetVolume(0.f);
	}
	else
	{
		mainMenuMusic->SetVolume((float)gameLoopScript->volume / 10.f);
	}
}
