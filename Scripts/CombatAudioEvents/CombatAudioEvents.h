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
	//Player
	ComponentAudioSource* stepSound = nullptr;
	ComponentAudioSource* bomb_take_off = nullptr;
	ComponentAudioSource* bomb_impact = nullptr;
	ComponentAudioSource* attack1 = nullptr;
	ComponentAudioSource* attack2 = nullptr;
	ComponentAudioSource* spin_attack = nullptr;
	ComponentAudioSource* drill_attack = nullptr;

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
