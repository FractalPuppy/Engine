#ifndef __ENEMYSTATEDEATH_H_
#define __ENEMYSTATEDEATH_H_

#include "EnemyState.h"

class GameObject;
class ComponentRenderer;

class EnemyStateDeath :
	public EnemyState
{
public:
	EnemyStateDeath(BuriedEnemyAIScript* AIScript);
	~EnemyStateDeath();

	void Enter() override;
	void Update() override;

private:
	void BonesExplosionFX();

	GameObject* bonesParent = nullptr;
	GameObject* bonesDeathFX = nullptr;

	std::vector<GameObject*> deathBones;
	std::vector<std::pair<math::float3, math::float3>> boneInfo;

	ComponentRenderer* renderer = nullptr;

	float bonesSpeed = 2000.0f;
	float rotationSpeed = 0.05f;
	float deathDuration = 0.5f;
};

#endif // __ENEMYSTATEDEATH_H_