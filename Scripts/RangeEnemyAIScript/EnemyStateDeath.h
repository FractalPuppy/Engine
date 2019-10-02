#ifndef __ENEMYSTATEDEATH_H_
#define __ENEMYSTATEDEATH_H_

#include "EnemyState.h"

class GameObject;
class ComponentRenderer;

enum class DEATHTYPE
{
	NORMAL,
	CRIT
};

class EnemyStateDeath :
	public EnemyState
{
public:
	EnemyStateDeath(RangeEnemyAIScript* AIScript);
	~EnemyStateDeath();

	void Enter() override;
	void Update() override;

private:
	void BonesExplosionFX();
	void RemainingBonesFX();
	void ShowRemainingBones();

public:
	DEATHTYPE deathType = DEATHTYPE::NORMAL;

private:
	//BONE MESHES:
	GameObject* bonesParent = nullptr;
	//Explosion
	GameObject* explosionBones = nullptr;
	//Normal
	GameObject* remainingBones = nullptr;

	//DEATH FX:
	GameObject* bonesDeathFX = nullptr;
	//Explosion
	GameObject* explosionFX = nullptr;
	//Normal
	GameObject* standardFX = nullptr;


	std::vector<GameObject*> deathBones;
	std::vector<std::pair<math::float3, math::float3>> boneInfo;

	ComponentRenderer* renderer = nullptr;

	float bonesSpeed = 1000.0f;
	float rotationSpeed = 20.f;
	float totalRotation = 0.0f;

	float deathDuration = 0.8f;
};

#endif // __ENEMYSTATEDEATH_H_