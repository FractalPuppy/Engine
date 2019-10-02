#include "EnemyStateDeath.h"

#include "GameObject.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleTime.h"

#include "ComponentRenderer.h"
#include "ComponentTransform.h"
#include "RangeEnemyAIScript.h"
#include "EnemyControllerScript.h"

#include "Math/float3.h"
#include "Algorithm/Random/LCG.h"



EnemyStateDeath::EnemyStateDeath(RangeEnemyAIScript* AIScript)
{
	enemy = AIScript;
	trigger = "Death";

	renderer = (ComponentRenderer*)enemy->gameobject->GetComponentInChildren(ComponentType::Renderer);

	bonesParent = enemy->App->scene->FindGameObjectByName("DeathBones", enemy->gameobject);
	bonesDeathFX = enemy->App->scene->FindGameObjectByName("DeathFX", enemy->gameobject);
	LCG rand;

	if (bonesParent != nullptr)
	{
		explosionBones = *bonesParent->children.begin();
		for (GameObject* bone : explosionBones->children)
		{
			deathBones.emplace_back(bone);
			boneInfo.emplace_back(std::make_pair(math::float3(rand.Float() * 2 - 1, rand.Float(), rand.Float() * 2 - 1),
				math::float3(rand.Float() * 360, rand.Float() * 360, rand.Float() * 360)));
		}
		remainingBones = *bonesParent->children.rbegin();
	}

	if (bonesDeathFX != nullptr)
	{
		explosionFX = *bonesDeathFX->children.begin();
		standardFX = *bonesDeathFX->children.rbegin();
	}
}

void EnemyStateDeath::Enter()
{
	if (enemy->enemyController->IsDeadCritOrSkill())
	{
		deathType = DEATHTYPE::CRIT;
	}
	GameObject* meshScene = enemy->gameobject->children.front();
	meshScene->children.front()->SetActive(false); //disactive bones with its trails
	auxTimer = 0.0f;

	switch (deathType)
	{
	case DEATHTYPE::NORMAL:
		remainingBones->SetActive(true);
		standardFX->SetActive(true);
		break;
	case DEATHTYPE::CRIT:
		explosionBones->SetActive(true);
		explosionFX->SetActive(true);
		break;
	}

	renderer->highlighted = false;

	for (auto cr : enemy->enemyController->myRenders)
	{
		cr->castShadows = false;
	}

}

void EnemyStateDeath::Update()
{

	float waitedTime = (timer - auxTimer);

	switch (deathType)
	{
	case DEATHTYPE::NORMAL:
		if (waitedTime > 0.2f)
		{
			renderer->dissolveAmount = 0.3f + waitedTime / (deathDuration * 10);
			if (standardFX != nullptr && standardFX->children.front()->isActive())
			{
				standardFX->children.front()->SetActive(false);  //Disable initial smoke
			}
			RemainingBonesFX();
		}
		if (waitedTime > deathDuration)
		{
			ShowRemainingBones();
			if (standardFX != nullptr && standardFX->isActive())
			{
				standardFX->SetActive(false);
			}
		}
		break;
	case DEATHTYPE::CRIT:
		if (waitedTime > deathDuration * 0.3f)
		{
			if (explosionFX != nullptr && explosionFX->isActive())
			{
				explosionFX->SetActive(false);
			}
		}
		if (waitedTime > deathDuration)
		{
			enemy->gameobject->SetActive(false);

			if (explosionBones != nullptr)
			{
				explosionBones->SetActive(false);
			}
		}
		else if (waitedTime > deathDuration * 0.1f)
		{
			if (explosionFX != nullptr && explosionFX->children.front()->isActive())
			{
				explosionFX->children.front()->SetActive(false);  //Disable initial smoke
			}
			renderer->dissolveAmount = waitedTime / deathDuration;
			BonesExplosionFX();
		}
		break;
	}
}

void EnemyStateDeath::BonesExplosionFX()
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

		deathBones[i]->GetComponent<ComponentRenderer>()->dissolveAmount += enemy->App->time->gameDeltaTime*0.8f;
	}
}

void EnemyStateDeath::RemainingBonesFX()
{
	for (GameObject* bone : remainingBones->children)
	{
		ComponentRenderer* boneRenderer = bone->GetComponent<ComponentRenderer>();
		boneRenderer->dissolveAmount = MAX(0, boneRenderer->dissolveAmount - enemy->App->time->gameDeltaTime*0.3f);
	}
}

void EnemyStateDeath::ShowRemainingBones()
{
	if (enemy->gameobject->isActive())
	{
		bonesParent->children.remove(remainingBones);
		enemy->App->scene->root->InsertChild(remainingBones);
		enemy->gameobject->SetActive(false);
	}
}