#ifndef __CHAINATTACKSKILL_H_
#define __CHAINATTACKSKILL_H_

#include "MeleeSkill.h"

enum class AttackNumber{FIRST, SECOND};
enum class NextInput{ATTACK, SKILL, MOVE, NONE};

class ChainAttackSkill :
	public MeleeSkill
{
public:
	ChainAttackSkill(PlayerMovement* PM, const char* trigger, ComponentBoxTrigger* attackBox);
	~ChainAttackSkill();

	void Start() override;
	//void Update() override;

	void NextChainAttack();

public:
	static const math::float3 attackingBoxSize;

protected:
	void UseSkill() override;
	void Reset() override;

private:
	void CheckInput() override;

private:
	AttackNumber attack = AttackNumber::FIRST;

	NextInput nextInput = NextInput::NONE;

};

#endif __CHAINATTACKSKILL_H_