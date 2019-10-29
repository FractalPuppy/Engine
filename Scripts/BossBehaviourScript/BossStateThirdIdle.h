#ifndef __BOSSSTATETHIRDIDLE_H_
#define __BOSSSTATETHIRDIDLE_H_

#include "BossState.h"
#include "Math/float3.h"

enum class Fist
{
	None,
	Left,
	Right
};

class BossStateThirdIdle :
	public BossState
{
public:
	BossStateThirdIdle(BossBehaviourScript* AIBoss);
	~BossStateThirdIdle();

	void HandleIA() override;
	void Update() override;

	void Enter() override;
	void Exit() override;

	Fist lastFist = Fist::None;
	Fist secondLastFist = Fist::None;
	Fist nextFist = Fist::None;

private:
	Fist ChooseNextFist();
};

#endif // __BOSSSTATETHIRDIDLE_H_
