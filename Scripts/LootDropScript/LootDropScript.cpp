#include "LootDropScript.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleTime.h"
#include "ResourcePrefab.h"

#include "GameObject.h"
#include "ComponentTransform.h"

#include "Item.h"

#include "imgui.h"
#include "JSON.h"

LootDropScript_API Script* CreateScript()
{
	LootDropScript* instance = new LootDropScript;
	return instance;
}

void LootDropScript::Start()
{
	itemListGO = App->scene->FindGameObjectByName("ItemsList");
	if (itemListGO == nullptr)
		LOG("Warning: ItemList GO couldn't be found, items will spawn with root as a parent.");
}

void LootDropScript::Update()
{
	if(spawned && animate)
	{
		// Move the spawned items to their target location
		if (timer <= animDuration)
		{
			// Update timer
			timer += App->time->gameDeltaTime;
			// Percent is a 0-1 float showing the percentage of time that has passed on our timer
			float percent = timer / animDuration;
			
			if (percent > 1.0f)
			{
				percent = 1.0f;
			}
			for (int i = 0; i < spawnedGOList.size(); ++i)
			{
				// Lerp between target position and current position
				math::float3 newPosition = math::Lerp(gameobject->transform->position, itemsPosTarget[i], percent);

				// Parabolic movement on Y axis  
				newPosition.y += -(4 * animHigh * percent * percent) + 4 * animHigh * percent;
	
				spawnedGOList[i]->transform->SetPosition(newPosition);
			}
		}	
	}
	else if (spawned)
	{
		//gameobject->RemoveComponent(*this);
		this->Enable(false);
	}
}

void LootDropScript::Expose(ImGuiContext* context)
{
	for (int i = 0; i < itemList.size(); ++i)
	{
		ImGui::PushID(i);
		ImGui::Text("Item %i: ", i + 1);
		char* itemName = new char[64];
		strcpy_s(itemName, strlen(itemList[i].c_str()) + 1, itemList[i].c_str());
		ImGui::InputText("Prefab Name", itemName, 64);
		itemList[i] = itemName;
		delete[] itemName;
		ImGui::PopID();
	}

	if (ImGui::Button("Add item"))
	{
		itemList.push_back("");
	}
	ImGui::SameLine();
	if (ImGui::Button("Delete item"))
	{
		itemList.pop_back();
	}

	ImGui::Separator();
	ImGui::Text("Jump animation:");
	ImGui::Checkbox("Animation", &animate);

	if (animate)
	{
		ImGui::DragFloat("Duration", &animDuration, 0.1f);
		ImGui::DragFloat("Jump high", &animHigh);
	}
}

void LootDropScript::Serialize(JSON_value* json) const
{
	json->AddInt("NumberOfItems", itemList.size());

	std::string name;
	for (int i = 0; i < itemList.size(); ++i)
	{
		name = "ItemPrefabName" + std::to_string(i);
		json->AddString(name.c_str(), itemList[i].c_str());
	}

	json->AddInt("animate", animate);
	json->AddFloat("animDuration", animDuration);
	json->AddFloat("animHigh", animHigh);
}

void LootDropScript::DeSerialize(JSON_value* json)
{
	int numItems = json->GetInt("NumberOfItems");

	std::string name;
	for (int i = 0; i < numItems; ++i)
	{
		name = "ItemPrefabName" + std::to_string(i);
		itemList.push_back(json->GetString(name.c_str(), ""));
	}

	animate = json->GetInt("animate");
	animDuration = json->GetFloat("animDuration", 0.8f);
	animHigh = json->GetFloat("animHigh", 200.0f);
}

void LootDropScript::DropItems()
{
	for (int i = 0; i < itemList.size(); ++i)
	{
		if (itemListGO != nullptr)
		{
			math::float3 position = gameobject->transform->position + positionOffset;
			GameObject* spawnedItem = App->scene->Spawn(itemList[i].c_str(), position, itemListGO->transform->rotation, itemListGO);
			spawnedItem->transform->Rotate(rotationOffset);
		}
		else
			App->scene->Spawn(itemList[i].c_str());
	}
}

void LootDropScript::DropItemsInCircle(float radius)
{
	// 2 * pi is full circle, divide it between items to drop
	float angleStep = (2 * 3.14f) / itemList.size();
	float angle = 0.0f;
	for (int i = 0; i < itemList.size(); ++i)
	{
		if (itemListGO != nullptr)
		{
			// Apply position offset
			math::float3 position = gameobject->transform->position + positionOffset;

			// Position in circle = PosCenter + radius*cos(angle)
			position = math::float3(position.x + radius * cos(angle), position.y, position.z + radius * sin(angle));
			GameObject* spawnedItem = nullptr;

			if (animate)
			{
				// Spawn item on center of GO
				spawnedItem = App->scene->Spawn(itemList[i].c_str(), gameobject->transform->position, itemListGO->transform->rotation, itemListGO);
				itemsPosTarget.push_back(position - spawnedItem->transform->position);
			}
			else
				spawnedItem = App->scene->Spawn(itemList[i].c_str(), position, itemListGO->transform->rotation, itemListGO);

			// Apply rotation offset
			spawnedItem->transform->Rotate(rotationOffset);
			angle += angleStep;

			// Add pointers to spawned items to list
			if (animate)
				spawnedGOList.push_back(spawnedItem);
		}
		else
			App->scene->Spawn(itemList[i].c_str());
	}
	spawned = true;
}

void LootDropScript::DropItemsAtPositions(float radius, std::vector<GameObject*> spawnPositionGOList)
{
	// pi is half of a circle, divide it between items to drop
	float angleStep = 3.14f / (itemList.size() - 1);
	float angle = 0.0f;
	for (int i = 0; i < itemList.size(); ++i)
	{
		if (itemListGO != nullptr)
		{
			// Get position to spawn prefab (if there is no GO on list, use this->GO position)
			math::float3 position = gameobject->transform->position;

			if (spawnPositionGOList.size() > i)
				position = spawnPositionGOList[i]->transform->GetGlobalPosition();

			GameObject* spawnedItem = nullptr;

			if (animate)
			{
				// Spawn item on center of GO
				spawnedItem = App->scene->Spawn(itemList[i].c_str(), gameobject->transform->position, itemListGO->transform->rotation, itemListGO);
				itemsPosTarget.push_back(position);
			}
			else
				spawnedItem = App->scene->Spawn(itemList[i].c_str(), position, itemListGO->transform->rotation, itemListGO);

			// Apply rotation offset
			spawnedItem->transform->Rotate(rotationOffset);
			angle += angleStep;

			// Add pointers to spawned items to list
			if(animate)
				spawnedGOList.push_back(spawnedItem);
		}
		else
			App->scene->Spawn(itemList[i].c_str());
	}
	spawned = true;
}
