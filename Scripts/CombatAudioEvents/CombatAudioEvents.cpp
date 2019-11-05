#include "CombatAudioEvents.h"
#include "Application.h"

#include "ModuleScene.h"
#include "GameObject.h"

#include "ComponentAudioSource.h"

CombatAudioEvents_API Script* CreateScript()
{
	CombatAudioEvents* instance = new CombatAudioEvents;
	return instance;
}

void CombatAudioEvents::Start()
{
	GameObject* GO = nullptr;

	
}

void CombatAudioEvents::OnAnimationEvent(std::string name)
{

	
}

void CombatAudioEvents::enemyGotHit(int i)
{
	float offset = randomOffset(0.3) - 0.15;
	if (enemy_got_hit != nullptr)
	{
		enemy_got_hit->SetPitch(0.9 + offset);
		enemy_got_hit->Play();
	}
}

float CombatAudioEvents::randomOffset(float max)
{
	return max;
	
}


