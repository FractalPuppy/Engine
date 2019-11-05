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


	//Enemy
	ComponentAudioSource* enemy_got_hit = nullptr;

public:
	void Start() override;
	void OnAnimationEvent(std::string name) override;

	void enemyGotHit(int i);

private:
	float randomOffset(float max);

};

extern "C" CombatAudioEvents_API Script* CreateScript();

#endif __CombatAudioEvents_h__
