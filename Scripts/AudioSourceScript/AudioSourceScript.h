#ifndef  __AudioSourceScript_h__
#define  __AudioSourceScript_h__

#include "BaseScript.h"


#ifdef AudioSourceScript_EXPORTS
#define AudioSourceScript_API __declspec(dllexport)
#else
#define AudioSourceScript_API __declspec(dllimport)
#endif

class ComponentAudioSource;

class AudioSourceScript_API AudioSourceScript : public Script
{
	void Start() override;
	void Update() override;

private:
	ComponentAudioSource* audioSource = nullptr;
};

#endif __AudioSourceScript_h__
