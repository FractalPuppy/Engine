#ifndef __ENEMYSTATEATTACKSPIN_H_
#define __ENEMYSTATEATTACKSPIN_H_

#include  "BasicEnemyAIScript/EnemyStateAttack.h"
#include "Algorithm/Random/LCG.h"

class ComponentRenderer;
class ResourceMaterial;
class ComponentAudioSource;

enum class MATERIALTYPE
{
	ROTATION,
	DEFAULT
};

class EnemyStateAttackSpin :
	public EnemyStateAttack
{
public:
	EnemyStateAttackSpin(BasicEnemyAIScript* AIScript);
	~EnemyStateAttackSpin();

	void HandleIA() override;
	void Update() override;
	void Enter() override;
	void Exit() override;

protected:

	void Attack() override;
	bool LessThanHalfHP() const;
	void PunchFX(bool active);
	void EnableSpin();
	void DisableSpin();
	void SpinBehaviour();
	void SpinBones(bool active);
	void ChangeToSpinMaterial(MATERIALTYPE type) const;

	ComponentAudioSource* spinSFX = nullptr;
	ComponentRenderer* enemyRenderer = nullptr;
	GameObject* dust = nullptr;
	GameObject* spinOff = nullptr;
	LCG lcg;

	ResourceMaterial* rotationMaterial = nullptr;
	ResourceMaterial* defaultMaterial = nullptr;

	float rotationSpeed = 20.0f;
	float spinDuration = 3.0f;
	float spinCooldown = 1.0f;

	float spinTimer = 0.0f;
	bool spinning = false;
	bool isOnCooldown = false;

	int baseDamage = 5.0f;
	int spinDamage = 2.0f;
};

#endif __ENEMYSTATEATTACKSPIN_H_