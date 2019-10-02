#ifndef __ENEMYSTATEATTACK_H_
#define __ENEMYSTATEATTACK_H_

#include "EnemyState.h"

class EnemyStateAttack :
	public EnemyState
{
public:
	EnemyStateAttack(RangeEnemyAIScript* AIScript);
	~EnemyStateAttack();

	void Enter() override;
	void HandleIA() override;
	void Update() override;

	void ShowGunFX();

	void ProcessGunFx();

private:
	bool projShot1 = false;
	bool projShot2 = false;
	bool projShot3 = false;
	GameObject* gunFX = nullptr;

	float gunTimer = 0.0f;
	float gunTotalTimer = 0.2f;
};

#endif __ENEMYSTATEATTACK_H_