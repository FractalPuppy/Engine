#ifndef __ENEMYSTATEMULTIPLY_H_
#define __ENEMYSTATEMULTIPLY_H_

#include "BasicEnemyAIScript\EnemyState.h"
#include <vector>
class BasicEnemyAIScript;
class GameObject;
class ComponentRenderer;
class ComponentAudioSource;

class EnemyStateMultiply :
	public EnemyState
{
public:
	EnemyStateMultiply(BasicEnemyAIScript* AIScript);
	~EnemyStateMultiply();

	void Enter() override;
	void HandleIA() override;
	void SetSpawnParticles(bool active);

	bool isMultiplying = false;
	bool haveSpawned = false;
	float multiplyTimer = 0.3f;
	float spawnTimer = 0.25f;

	ComponentRenderer* meshInstance1 = nullptr;
	ComponentRenderer* meshInstance2 = nullptr;
private:
	bool hasMultiplied = false;
	std::vector<GameObject*> spawnParticleSystems;
	ComponentAudioSource* multiplySFX = nullptr;
};

#endif __ENEMYSTATEMULTIPLY_H_
