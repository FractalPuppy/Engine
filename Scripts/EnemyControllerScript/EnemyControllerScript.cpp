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
#include "CombatAudioEvents.h"
#include "LootDropScript.h"
#include "WorldControllerScript.h"

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
	//add the enemy to the world controller script
	//this should be called everytime levels are switched
	
	if (gameobject->tag != "Boss")
	{
		currentWorldControllerScript = App->scene->FindGameObjectByName("WorldController")->GetComponent<WorldControllerScript>();
		currentWorldControllerScript->addEnemy(gameobject);
	}
	
}

void EnemyControllerScript::Awake()
{
	std::vector<Component*> renders = gameobject->GetComponentsInChildren(ComponentType::Renderer);
	for (std::vector<Component*>::iterator it = renders.begin(); it != renders.end(); ++it)
		myRenders.push_back((ComponentRenderer*)(*it));

	if (myRenders.size() > 0u)
	{
		// Look for enemy BBox (Will take first found)
		myBbox = &myRenders.at(0)->gameobject->bbox;
		if (myBbox == nullptr)
		{
			LOG("The enemy %s has no bbox \n", gameobject->name);
		}

		// Get playerMesh
		myMesh = myRenders.at(0)->gameobject;
	}
	else
	{
		LOG("Error: The enemy mesh couldn't be found.");
	}

	// Get hit material
	std::vector<std::string> materials = App->resManager->GetResourceNamesList(TYPE::MATERIAL, true);
	for (std::string matName : materials)
	{
		if (matName == hitMaterialName)
		{
			hitMaterial = (ResourceMaterial*)App->resManager->GetByName(matName.c_str(), TYPE::MATERIAL);
		}
	}
	// Will only change first renderer material on hit
	for (ComponentRenderer* renderer : myRenders)
	{
		defaultMaterials.push_back(renderer->material);
	}

	if (!hitMaterial)
	{
		hitMaterial = (ResourceMaterial*)App->resManager->GetByName(DEFAULTMAT, TYPE::MATERIAL);
	}

	// Look for player and his BBox
	player = App->scene->FindGameObjectByTag(playerTag.c_str());
	if (player == nullptr)
	{
		LOG("The Player GO with tag %s couldn't be found \n", playerTag.c_str());
	}
	else
	{
		playerHitBox = player->GetComponent<ComponentBoxTrigger>();
		if (playerHitBox == nullptr)
		{
			LOG("The GameObject %s has no ComponentTrigger attached \n", player->name.c_str());
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
		if (enemyLifeBar == nullptr)
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

	// Look for LootDropScript
	lootDrop = gameobject->GetComponent<LootDropScript>();
}

void EnemyControllerScript::Update()
{
	math::float3 closestPoint;
	fPoint mouse_point = App->input->GetMousePosition();
	math::float2 mouse = { mouse_point.x, mouse_point.y };
	std::list<GameObject*> intersects = App->scene->SceneRaycastHit(mouse);

	if (playerMovement->isPlayerDead) return;

	auto mesh = std::find(intersects.begin(), intersects.end(), this->myMesh);
	if(mesh != std::end(intersects) && *mesh == this->myMesh)
	{
		// Show enemy lifebar
		if (enemyLifeBar != nullptr)
		{
			switch (enemyType)
			{
			default:
			case EnemyType::SKELETON:	enemyLifeBar->SetLifeBar(maxHealth, actualHealth, EnemyLifeBarType(enemyLevel), "Skeleton");	break;
			case EnemyType::MINER:		enemyLifeBar->SetLifeBar(maxHealth, actualHealth, EnemyLifeBarType(enemyLevel), "Miner"); 		break;
			case EnemyType::SORCERER:	enemyLifeBar->SetLifeBar(maxHealth, actualHealth, EnemyLifeBarType(enemyLevel), "Sorcerer");	break;
			case EnemyType::SPINNER:	enemyLifeBar->SetLifeBar(maxHealth, actualHealth, EnemyLifeBarType(enemyLevel), "Spinner"); 	break;
			case EnemyType::BANDOLERO:	enemyLifeBar->SetLifeBar(maxHealth, actualHealth, EnemyLifeBarType(enemyLevel), "Bandolero");	break;
			}		
		}

		if (myRenders.size() > 0u && !isDead)
		{
			for (std::vector<ComponentRenderer*>::iterator it = myRenders.begin(); it != myRenders.end(); ++it)
				(*it)->highlighted = true;
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
		if (myRenders.size() > 0u)
		{
			for (std::vector<ComponentRenderer*>::iterator it = myRenders.begin(); it != myRenders.end(); ++it)
				(*it)->highlighted = false;

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
		// Set default material back to all meshes
		for (unsigned i = 0u; i < myRenders.size(); i++)
		{
			myRenders.at(i)->material = defaultMaterials.at(i);
		}
		enemyHit = false;
	}

	if (isDead && !lootDropped)
	{
		if (deathTimer > lootDelay)
		{
			if (lootDrop != nullptr)
			{
				// If chest has more than one item drop them in circle
				if (lootDrop->itemList.size() > 1)
					lootDrop->DropItemsInCircle(lootRadius);
				else
					lootDrop->DropItems();
			}
			lootDropped = true;
		}
		else
		{
			deathTimer += App->time->gameDeltaTime;
		}
	}
	if (isDead && !removedFromCrowd)
	{
		//remove the enemy from the crowd
		currentWorldControllerScript->RemoveEnemy(gameobject->UUID);
		removedFromCrowd = true;
	}
}

void EnemyControllerScript::Expose(ImGuiContext* context)
{

	// Enemy Type
	const char* types[] = { "Skeleton", "Miner", "Sorcerer", "Spinner", "Bandolero" };
	if (ImGui::BeginCombo("Type", types[(int)enemyType]))
	{
		for (int n = 0; n < 5; n++)
		{
			bool isSelected = ((int)enemyType == n);
			if (ImGui::Selectable(types[n], isSelected) && (int)enemyType != n)
			{
				enemyType = (EnemyType)n;
			}
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	ImGui::SliderInt("Level", &enemyLevel, 1, 3);

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

	// Draw the name of every GO that has a ComponentRenderer 
	if (myRenders.size() > 0u)
	{
		ImGui::Text("Renderers:");
		for (std::vector<ComponentRenderer*>::iterator it = myRenders.begin(); it != myRenders.end(); ++it)
			ImGui::Text(((Component*)(*it))->gameobject->name.c_str());
	}
	ImGui::Separator();
	ImGui::Text("Loot Variables:");
	ImGui::DragFloat("Loot Delay", &lootDelay);
	ImGui::DragFloat("Loot Radius", &lootRadius);
}

void EnemyControllerScript::Serialize(JSON_value* json) const
{
	assert(json != nullptr);
	json->AddInt("type", (int)enemyType);
	json->AddInt("level", enemyLevel);
	json->AddString("playerTag", playerTag.c_str());
	json->AddInt("health", maxHealth);
	json->AddInt("experience", experience);
	json->AddString("enemyCursor", enemyCursor.c_str());
	json->AddFloat("lootDelay", lootDelay);
	json->AddFloat("lootRadius", lootRadius);
}

void EnemyControllerScript::DeSerialize(JSON_value* json)
{
	assert(json != nullptr);
	enemyType = (EnemyType)json->GetInt("type");
	enemyLevel = json->GetInt("level");
	playerTag = json->GetString("playerTag", "Player");
	maxHealth = json->GetInt("health", maxHealth);
	experience = json->GetInt("experience", 20);
	actualHealth = maxHealth;
	enemyCursor = json->GetString("enemyCursor", "RedGlow.cur");
	lootDelay = json->GetFloat("lootDelay", 1.0f);
	lootRadius = json->GetFloat("lootRadius", 100.0f);
}

void EnemyControllerScript::TakeDamage(unsigned damage, int type)
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
			gameobject->SetActive(false);
		}
		else
		{
			actualHealth -= damage;
			// Set hit material to all enemy meshes
			for (unsigned i = 0u; i < myRenders.size(); i++)
			{
				myRenders.at(i)->material = hitMaterial;
			}
			hitColorTimer = hitColorDuration;
		}

		if (actualHealth <= 0)
		{
			isDead = true;
			if ((DamageType)type == DamageType::CRITICAL || playerMovement->IsExecutingSkill())
			{
				isDeadByCritOrSkill = true; //by default is false (Normal)
			}

			if (experienceController != nullptr)
				experienceController->AddXP(experience);

			// Disable hit boxes
			hpBoxTrigger->Enable(false);
			
			if (attackBoxTrigger != nullptr)
			{
				attackBoxTrigger->Enable(false);
			}

			// Unhighlight
			if (myRenders.size() > 0u)
			{
				for (std::vector<ComponentRenderer*>::iterator it = myRenders.begin(); it != myRenders.end(); ++it)
					(*it)->highlighted = false;
			}
		}
		damageController->AddDamage(gameobject->transform, damage, (DamageType)type);
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
	return gameobject->transform->GetGlobalRotation();
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

inline ComponentRenderer* EnemyControllerScript::GetMainRenderer() const
{
	if (myRenders.size() > 0u)
		return myRenders.at(0u);
	else
		return nullptr;
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

bool EnemyControllerScript::Move(float speed, float& refreshTime, math::float3 position, std::vector<float3>& path) const
{
	if (speed != currentSpeed)
	{
		//update our
		currentSpeed = speed;
		//now we change the corresponding agent's speed
		currentWorldControllerScript->changeVelocity(gameobject->UUID, currentSpeed);
	}
	currentWorldControllerScript->EnemyMoveRequest(gameobject->UUID, position);
	//gameobject->transform->LookAt(gameobject->transform->movingOrientation);
	return true;
}

bool EnemyControllerScript::IsIdle() const
{
	return currentWorldControllerScript->IsAgentIdle(gameobject->UUID);
}

bool EnemyControllerScript::IsStuck() const
{
	return currentWorldControllerScript->IsAgentStuck(gameobject->UUID);
}

void EnemyControllerScript::Stop()
{
	currentWorldControllerScript->StopAgent(gameobject->UUID);
}

void EnemyControllerScript::LookAt2D(const math::float3& position)
{
	math::float3 auxPos = position;
	auxPos.y = GetPosition().y;
	gameobject->transform->LookAtLocal(auxPos);
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

	if (go->tag == "PlayerHitBoxAttack" || go->tag == "Machete")
	{
		if (gameobject->tag.c_str() != "Boss")
		{
			// Generate a random number and if it is below the critical chance the damage will be increased
			if ((rand() % 100u) < playerMovement->criticalChance)
			{
				TakeDamage(playerMovement->stats.strength * 0.2f, (int)DamageType::CRITICAL);
			}
			else
			{
				TakeDamage(playerMovement->stats.strength * 0.1f, (int)DamageType::NORMAL);
			}
		}
		//else
		//{
		//	float distanceToPlayer = GetDistanceToPlayer2D();
		//	if (distanceToPlayer > 500.0f)
		//	{

		//	}
		//}

	}
}
