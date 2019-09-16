#ifndef  __MenuSoundsScript_h__
#define  __MenuSoundsScript_h__

#include "BaseScript.h"
#include <vector>

#ifdef MenuSoundsScript_EXPORTS
#define MenuSoundsScript_API __declspec(dllexport)
#else
#define MenuSoundsScript_API __declspec(dllimport)
#endif

class Component;
class Button;
class ComponentAudioSource;
class GameLoop;

class MenuSoundsScript_API MenuSoundsScript : public Script
{
public:
	void Start() override;
	void Update() override;

	inline virtual MenuSoundsScript* Clone() const
	{
		return new MenuSoundsScript(*this);
	}
public:

	std::vector<Component*> buttons;

	GameObject* Menu = nullptr;

	ComponentAudioSource* audioClick = nullptr;
	ComponentAudioSource* audioHovered = nullptr;
	ComponentAudioSource* ambienceMusic = nullptr;
	ComponentAudioSource* mainMenuMusic = nullptr;
private:

	bool itemHovered = false;
	bool itemClicked = false;

	GameLoop* gameLoopScript = nullptr;

};

extern "C" MenuSoundsScript_API Script* CreateScript();

#endif __MenuSoundsScript_h__
