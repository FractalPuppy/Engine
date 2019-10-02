#ifndef __PLAYERSTATEATTACK_H_
#define __PLAYERSTATEATTACK_H_

#include "PlayerState.h"

class PlayerMovement;
class BasicSkill;
enum class SkillKey;

class PlayerStateAttack :
	public PlayerState
{
public:
	PlayerStateAttack(PlayerMovement* PM, const char* trigger,
		math::float3 boxSize, float minTime, float maxTime);
	virtual ~PlayerStateAttack();

	void Enter() override;
	void Update() override;


public:
	SkillKey skill;
};

#endif __PLAYERSTATEATTACK_H_
