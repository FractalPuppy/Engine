#ifndef  __CombatAudioEvents_h__
#define  __CombatAudioEvents_h__

#include "BaseScript.h"

#ifdef CombatAudioEvents_EXPORTS
#define CombatAudioEvents_API __declspec(dllexport)
#else
#define CombatAudioEvents_API __declspec(dllimport)
#endif


class ComponentAudioSource;

class CombatAudioEvents_API CombatAudioEvents : public Script
{
public:
	ComponentAudioSource* m1AttackSound = nullptr;
	ComponentAudioSource* stepSound = nullptr;

public:
	void Start() override;
	void OnAnimationEvent(std::string name) override;

};

extern "C" CombatAudioEvents_API Script* CreateScript();

#endif __CombatAudioEvents_h__
