#include "EnemyControllerScript.h"

#include <algorithm>

#include "Application.h"
#include "ModuleInput.h"
#include "ModuleScene.h"
#include "ModuleScript.h"
#include "ModuleTime.h"
#include "ModuleNavigation.h"
#include "ModuleResourceManager.h"
#include "MouseController.h"

#include "GameObject.h"
#include "ComponentRenderer.h"
#include "ComponentTransform.h"
#include "ComponentBoxTrigger.h"

#include "PlayerMovement.h"
#include "ResourceMaterial.h"
#include "ExperienceController.h"
#include "DamageController.h"
#include "EnemyLifeBarController.h"
#include "EnemyLoot.h"
#include "CombatAudioEvents.h"

#include "imgui.h"
#include "JSON.h"



#define MINIMUM_PATH_DISTANCE 400.0f
#define MOVE_REFRESH_TIME 0.3f

EnemyControllerScript_API Script* CreateScript()
{
	EnemyControllerScript* instance = new EnemyControllerScript;
	return instance;
}

void EnemyControllerScript::Start()
{
}

void EnemyControllerScript::Awake()
{
	myRender = (ComponentRenderer*)gameobject->GetComponentInChildren(ComponentType::Renderer);
	if (myRender != nullptr)
	{
		// Look for enemy BBox
		myBbox = &myRender->gameobject->bbox;
		if (myBbox == nullptr)
		{
			LOG("The enemy %s has no bbox \n", gameobject->name);
		}

		// Get playerMesh
		myMesh = myRender->gameobject;
	}
	else
	{
		LOG("Error: The enemy mesh couldn't be found.");
	}

	// Look for player and his BBox
	player = App->scene->FindGameObjectByTag(playerTag.c_str());
	if (player == nullptr)
	{
		LOG("The Player GO with tag %s couldn't be found \n", playerTag);
	}
	else
	{
		playerHitBox = player->GetComponent<ComponentBoxTrigger>();
		if (playerHitBox == nullptr)
		{
			LOG("The GameObject %s has no bbox attached \n", player->name);
		}

		playerMovement = (PlayerMovement*)player->GetComponentInChildren(ComponentType::Script);
	}

	// Look for Component Animation of the enemy
	anim = (ComponentAnimation*)gameobject->GetComponentInChildren(ComponentType::Animation);
	if (anim == nullptr)
	{
		LOG("No child of the GameObject %s has an Animation component attached \n", gameobject->name.c_str());
	}

	GameObject* damageGO = App->scene->FindGameObjectByName("Damage");
	if (damageGO == nullptr)
	{
		LOG("Damage controller GO couldn't be found \n");
	}
	else
	{
		damageController = damageGO->GetComponent<DamageController>();
		if (damageController == nullptr)
		{
			LOG("Damage controller couldn't be found \n");
		}
	}

	GameObject* enemyLifeGO = App->scene->FindGameObjectByName("EnemyLife");
	if (enemyLifeGO == nullptr)
	{
		LOG("Enemy controller GO couldn't be found \n");
	}
	else
	{
		enemyLifeBar = enemyLifeGO->GetComponent<EnemyLifeBarController>();
		if (enemyLifeBar != nullptr)
		{
			LOG("Damage controller couldn't be found \n");
		}
	}

	hpBoxTrigger = (ComponentBoxTrigger*)gameobject->GetComponentInChildren(ComponentType::BoxTrigger);
	if (hpBoxTrigger == nullptr)
	{
		LOG("No child of the GameObject %s has a boxTrigger component attached \n", gameobject->name.c_str());
	}

	GameObject* attackGameObject = App->scene->FindGameObjectByName("HitBoxAttack", gameobject);
	//assert(attackGameObject != nullptr);
	if(attackGameObject != nullptr)
	{
		attackBoxTrigger = (ComponentBoxTrigger*)attackGameObject->GetComponentInChildren(ComponentType::BoxTrigger);
		if (attackBoxTrigger == nullptr)
		{
			LOG("No child of the GameObject %s has a boxTrigger component attached \n", attackGameObject->name.c_str());
		}
		else
		{
			attackBoxTrigger->Enable(false);
		}
	}
	

	GameObject* xpGO = App->scene->FindGameObjectByName("Xp");
	if (xpGO == nullptr)
	{
		LOG("Xp controller GO couldn't be found \n");
	}
	else
	{
		experienceController = xpGO->GetComponent<ExperienceController>();
		if (experienceController == nullptr)
		{
			LOG("experienceController couldn't be found \n");
		}
	}
	std::vector<std::string> materials = App->resManager->GetResourceNamesList(TYPE::MATERIAL, true);
	for (std::string matName : materials)
	{
		if (matName == hitMaterialName)
		{
			hitMaterial = (ResourceMaterial*)App->resManager->GetByName(matName.c_str(), TYPE::MATERIAL);
		}
	}
	defaultMaterial = myRender->material;
	if (!hitMaterial)
	{
		hitMaterial = defaultMaterial;
	}

	GameObject* playerGO = App->scene->FindGameObjectByName("Player");
	if (playerGO == nullptr)
	{
		LOG("Player couldn't be found \n");
	}
	else
	{
		combataudioevents = playerGO->GetComponent<CombatAudioEvents>();
		if (combataudioevents == nullptr)
		{
			LOG("combataudioevents couldn't be found \n");
		}
	}
}

void EnemyControllerScript::Update()
{
	math::float3 closestPoint;
	fPoint mouse_point = App->input->GetMousePosition();
	math::float2 mouse = { mouse_point.x, mouse_point.y };
	std::list<GameObject*> intersects = App->scene->SceneRaycastHit(mouse);

	auto mesh = std::find(intersects.begin(), intersects.end(), this->myMesh);
	if(mesh != std::end(intersects) && *mesh == this->myMesh)
	{
		if(enemyLifeBar != nullptr)
			enemyLifeBar->SetLifeBar(maxHealth, actualHealth, EnemyLifeBarType::NORMAL, "Skeleton");

		if (myRender != nullptr && !isDead)
		{
			myRender->highlighted = true;
		}

		//we need to keep track of current targeted enemy
		App->scene->enemyHovered.object = gameobject;
		App->scene->enemyHovered.health = actualHealth;

		if (App->scene->enemyHovered.object != nullptr &&
			gameobject->UUID == App->scene->enemyHovered.object->UUID)
		{
			MouseController::ChangeCursorIcon(enemyCursor);
		}
	}
	else
	{
		if (myRender != nullptr)
		{
			myRender->highlighted = false;

			//if this is the enemy that was being targeted, we untarget it from the scene
			if (App->scene->enemyHovered.object != nullptr &&
				gameobject->UUID == App->scene->enemyHovered.object->UUID)
			{
				App->scene->enemyHovered.object = nullptr;
				App->scene->enemyHovered.health = 0;
				MouseController::ChangeCursorIcon(gameStandarCursor);
			}
		}
	}

	if (enemyHit && hitColorTimer > 0.f)
	{
		hitColorTimer -= App->time->gameDeltaTime;
	}
	else if (enemyHit)
	{
		enemyHit = false;
		myRender->material = defaultMaterial;
	}
}

void EnemyControllerScript::Expose(ImGuiContext* context)
{
	if (ImGui::InputInt("Health", &maxHealth))
	{
		actualHealth = maxHealth;
	}

	ImGui::InputInt("Experience", &experience);

	ImGui::Separator();
	ImGui::Text("Player:");
	char* goName = new char[64];
	strcpy_s(goName, strlen(playerTag.c_str()) + 1, playerTag.c_str());
	ImGui::InputText("playerTag", goName, 64);
	playerTag = goName;
	delete[] goName;

	ImGui::Separator();
	ImGui::Text("Enemy cursor:");
	char* enemyCursorAux = new char[64];
	strcpy_s(enemyCursorAux, strlen(enemyCursor.c_str()) + 1, enemyCursor.c_str());
	ImGui::InputText("enemyCursor", enemyCursorAux, 64);
	enemyCursor = enemyCursorAux;
	delete[] enemyCursorAux;
	
}

void EnemyControllerScript::Serialize(JSON_value* json) const
{
	assert(json != nullptr);
	json->AddString("playerTag", playerTag.c_str());
	json->AddInt("health", maxHealth);
	json->AddInt("experience", experience);
	json->AddString("enemyCursor", enemyCursor.c_str());
}

void EnemyControllerScript::DeSerialize(JSON_value* json)
{
	assert(json != nullptr);
	playerTag = json->GetString("playerTag", "Player");
	maxHealth = json->GetInt("health", maxHealth);
	experience = json->GetInt("experience", 20);
	actualHealth = maxHealth;
	enemyCursor = json->GetString("enemyCursor", "RedGlow.cur");
}

void EnemyControllerScript::TakeDamage(unsigned damage)
{
	if (!isDead)
	{
		if (combataudioevents != nullptr)
		{
			combataudioevents->enemyGotHit(0);
		}
		enemyHit = true;
		if (actualHealth - damage < 0 )
		{
			actualHealth = 0;
			enemyLoot = gameobject->GetComponent<EnemyLoot>();
			if (enemyLoot != nullptr)
			{
				enemyLoot->GenerateLoot();
			}
			gameobject->SetActive(false);
			
		}
		else
		{
			actualHealth -= damage;
			myRender->material = hitMaterial;
			hitColorTimer = hitColorDuration;
		}

		if (actualHealth <= 0)
		{
			isDead = true;
			enemyLoot = gameobject->GetComponent<EnemyLoot>();
			if (enemyLoot != nullptr)
			{
				enemyLoot->GenerateLoot();
			}
			if (experienceController != nullptr)
				experienceController->AddXP(experience);
		}
		damageController->AddDamage(gameobject->transform, damage, 2);
	}
}

inline math::float3 EnemyControllerScript::GetPosition() const
{
	assert(gameobject->transform != nullptr);
	return gameobject->transform->GetGlobalPosition();
}

inline math::Quat EnemyControllerScript::GetRotation() const
{
	assert(gameobject->transform != nullptr);
	return gameobject->transform->GetRotation();
}

inline math::float3 EnemyControllerScript::GetPlayerPosition() const
{
	assert(player->transform != nullptr);
	return player->transform->GetGlobalPosition();
}

inline void EnemyControllerScript::SetPosition(math::float3 newPos) const
{
	assert(gameobject->transform != nullptr);
	gameobject->transform->SetGlobalPosition(newPos);
}

inline float EnemyControllerScript::GetDistanceTo(math::float3& position) const
{
	math::float3 enemyDistance = GetPosition();
	return enemyDistance.Distance(position);
}

inline float EnemyControllerScript::GetDistanceTo2D(math::float3& position) const
{
	math::float3 enemyDistance = GetPosition();
	enemyDistance.y = position.y;
	return enemyDistance.Distance(position);
}

inline float EnemyControllerScript::GetDistanceToPlayer2D() const
{
	math::float3 enemyPosition = GetPosition();
	math::float3 playerPosition = GetPlayerPosition();
	enemyPosition.y = playerPosition.y;
	return enemyPosition.Distance(playerPosition);
}

inline bool EnemyControllerScript::IsCollidingWithPlayer() const
{
	assert(myBbox != nullptr && playerHitBox != nullptr);
	return myBbox->Intersects(*playerHitBox->GetOBB());
}

void EnemyControllerScript::Move(float speed, math::float3& direction) const
{
	math::float3 movement = direction.Normalized() * speed * App->time->gameDeltaTime;
	gameobject->transform->SetPosition(gameobject->transform->GetPosition() + movement);
}

void EnemyControllerScript::Move(float speed, float& refreshTime, math::float3 position, std::vector<float3>& path) const
{
	if (refreshTime > MOVE_REFRESH_TIME)
	{
		refreshTime = 0.0f;
		App->navigation->FindPath(gameobject->transform->position, position, path);
	}
	if (path.size() > 0)
	{
		math::float3 currentPosition = gameobject->transform->GetPosition();
		while (path.size() > 0 && currentPosition.DistanceSq(path[0]) < MINIMUM_PATH_DISTANCE)
		{
			path.erase(path.begin());
		}
		if (path.size() > 0)
		{
			gameobject->transform->LookAt(path[0]);
			math::float3 direction = (path[0] - currentPosition).Normalized();
			gameobject->transform->SetPosition(currentPosition + speed * direction * App->time->gameDeltaTime);
		}
	}
	refreshTime += App->time->gameDeltaTime;
}

void EnemyControllerScript::LookAt2D(math::float3& position)
{
	math::float3 auxPos = position;
	auxPos.y = GetPosition().y;
	gameobject->transform->LookAt(auxPos);
}

void EnemyControllerScript::OnTriggerEnter(GameObject* go)
{
	if (go == playerHitBox->gameobject)
	{
		auto overlaper = attackBoxTrigger->overlapList.find(playerHitBox);
		if (overlaper != attackBoxTrigger->overlapList.end() && overlaper->second == OverlapState::PostIdle)
		{
			playerMovement->Damage(5);
		}
	}

	if (go != lastHittedGO && go->tag == "PlayerHitBoxAttack")
	{
		TakeDamage(playerMovement->stats.strength * 0.1);
		lastHittedGO = go;
	}
}

void EnemyControllerScript::OnTriggerExit(GameObject* go)
{
	if (go == lastHittedGO)
	{
		lastHittedGO = nullptr;
	}
}