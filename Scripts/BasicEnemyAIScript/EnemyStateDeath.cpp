#include "EnemyStateDeath.h"

#include "GameObject.h"
#include "BasicEnemyAIScript.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleTime.h"

#include "ComponentRenderer.h"
#include "ComponentTransform.h"

#include "Math/float3.h"
#include "Algorithm/Random/LCG.h"

EnemyStateDeath::EnemyStateDeath(BasicEnemyAIScript* AIScript)
{
	enemy = AIScript;
	trigger = "Idle";

	renderer = (ComponentRenderer*) enemy->gameobject->GetComponentInChildren(ComponentType::Renderer);

	bonesParent = enemy->App->scene->FindGameObjectByName("DeathBones", enemy->gameobject);
	bonesDeathFX = enemy->App->scene->FindGameObjectByName("DeathFX", enemy->gameobject);
	LCG rand;

	if (bonesParent != nullptr)
	{
		for (GameObject* bone : bonesParent->children)
		{
			deathBones.emplace_back(bone);
			boneInfo.emplace_back(std::make_pair(math::float3(rand.Float() * 2 - 1, rand.Float(), rand.Float() * 2 - 1),
				math::float3(rand.Float() * 360, rand.Float() * 360, rand.Float() * 360)));
		}
	}
}


EnemyStateDeath::~EnemyStateDeath()
{
}

void EnemyStateDeath::Enter()
{
	GameObject* meshScene = enemy->gameobject->children.front();
	meshScene->children.front()->SetActive(false); //disactive bones with its trails
	auxTimer = 0.0f;
	if (bonesParent != nullptr)
	{
		bonesParent->SetActive(true);
		bonesDeathFX->SetActive(true);
	}
	renderer->highlighted = false;
	
}

void EnemyStateDeath::Update()
{
	float waitedTime = (timer - auxTimer);

	if (waitedTime > deathDuration * 0.3f)
	{
		if (bonesDeathFX != nullptr && bonesDeathFX->isActive())
		{
			bonesDeathFX->SetActive(false);
		}
	}

	if (waitedTime > deathDuration)
	{
		enemy->gameobject->SetActive(false);
		if (bonesParent != nullptr)
		{
			bonesParent->SetActive(false);
		}
	}
	else if(waitedTime > deathDuration*0.1f)
	{
		if (bonesDeathFX != nullptr && bonesDeathFX->children.front()->isActive())
		{
			bonesDeathFX->children.front()->SetActive(false);
		}
		renderer->dissolveAmount = waitedTime / deathDuration;
		DeathAnimation();
	}
}

void EnemyStateDeath::DeathAnimation()
{
	for (unsigned i = 0; i < deathBones.size(); ++i)
	{
		math::float3 currentPosition = deathBones[i]->transform->GetPosition();
		deathBones[i]->transform->SetPosition(currentPosition + boneInfo[i].first * bonesSpeed * enemy->App->time->gameDeltaTime);

		//math::Quat currentRotation = deathBones[i]->transform->GetRotation();
		//deathBones[i]->transform->SetRotation(currentRotation.Mul(
		//	Quat::FromEulerXYZ(boneInfo[i].second.x* rotationSpeed * enemy->App->time->gameDeltaTime, 
		//		boneInfo[i].second.y * rotationSpeed * enemy->App->time->gameDeltaTime,
		//		boneInfo[i].second.z * rotationSpeed * enemy->App->time->gameDeltaTime)));

		deathBones[i]->GetComponent<ComponentRenderer>()->dissolveAmount += enemy->App->time->gameDeltaTime;
	}
}
