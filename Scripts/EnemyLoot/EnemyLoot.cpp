#include "EnemyLoot.h"
#include "Application.h"
#include "Resource.h"
#include "ModuleScene.h"
#include "ModuleScript.h"
#include "ModuleResourceManager.h"

#include "ComponentTransform.h"

#include "GameObject.h"

#include "ItemPicker.h"

#include "Math/float4x4.h"
#include <algorithm>
#include "JSON.h"


#pragma warning(disable : 4996)

EnemyLoot_API Script* CreateScript()
{
	EnemyLoot* instance = new EnemyLoot;
	return instance;
}

void EnemyLoot::Start()
{
	/*std::list<GameObject*> GOs = gameobject->children;

	for (std::list<GameObject*>::iterator it = GOs.begin(); it != GOs.end(); ++it)
	{
		if ((*it)->GetComponent<ItemPicker>() != nullptr && (std::find_if(items.begin(), items.end(),
			[&it](const std::pair<GameObject*, int>& element) { return element.first == (*it); })) != items.end())
		{
			items.push_back(std::make_pair((*it), 100));
		}
	}*/
}

void EnemyLoot::Update()
{
}

void EnemyLoot::GenerateLoot()
{
	for (int i = 0; i < items.size(); ++i)
	{
		go = items[i].first;
		if ((rand() % 100) < items[i].second)
		{
			go->SetActive(true);
			math::float3 pos = gameobject->transform->GetGlobalPosition();
			float x = static_cast <float>(rand() / static_cast <float> (RAND_MAX / 200));
			float z = static_cast <float>(rand() / static_cast <float> (RAND_MAX / 200));
			pos.x += x;
			pos.y += 30;
			pos.z += z;
			go->transform->SetGlobalPosition(pos);
			go->ChangeParent(gameobject, list);

			go->movedFlag = true;
		}
	}
	items.clear();
}


void EnemyLoot::Expose(ImGuiContext* context)
{
	if (!actLoot.empty())
	{
		for (int i = 0; i < actLoot.size(); ++i)
		{
			GameObject* act = App->scene->FindGameObjectByName(actLoot[i].first.c_str(), gameobject);
			items.push_back(std::make_pair(act, actLoot[i].second));
		}
		actLoot.clear();
	}
	if (list == nullptr)
	{
		list = App->scene->FindGameObjectByName("ItemsList");
	}
	ImGui::SetCurrentContext(context);
	ImGui::Separator();
	ImGui::Text("List of items to drop:");
	for (int i = 0; i < items.size(); ++i)
	{
		ImGui::Text("Item: ");
		ImGui::SameLine();
		ImGui::Text(items[i].first->name.c_str());
		ImGui::SameLine();
		ImGui::Separator();
		ImGui::SameLine();
		ImGui::Text("Drop:");
		ImGui::SameLine();
		ImGui::Text(std::to_string(items[i].second).c_str());

	}
	if (items.size() == 0)
	{
		ImGui::Text("No items!");
	}

	GetItems();

	if (itemList.size() > 0 && ImGui::BeginCombo("Item name", itemName.c_str()))
	{
		for (int i = 0; i < itemList.size(); i++)
		{
			if (ImGui::Selectable(itemList[i].c_str()))
			{
				itemName = itemList[i];
			}
		}
		ImGui::EndCombo();
	}

	ImGui::DragInt("Drop %", &drop, 0.01f, 0, 100);

	if(ImGui::Button("Add item"))
	{
		go = App->scene->FindGameObjectByName(itemName.c_str());
		go->ChangeParent(list, gameobject);
		items.push_back(std::make_pair(go, drop));
		if (go->isActive())
		{
			go->SetActive(false);
		}
		goName = "GO Name";
		itemName = "";
		uid = 0;
		drop = 0;
	}

	ImGui::SameLine();

	if (ImGui::Button("Clear"))
	{
		for (int i = 0; i < items.size(); ++i)
		{
			go = items[i].first;
			go->ChangeParent(gameobject, list);
			go->SetActive(true);
		}
		items.clear();
	}
}


void EnemyLoot::GetItems()
{
	itemList.clear();
	std::vector<std::string> items;
	std::list<GameObject*> GOs = list->children;

	for (std::list<GameObject*>::iterator it = GOs.begin(); it != GOs.end(); ++it)
	{
		if ((*it)->GetComponent<ItemPicker>() != nullptr)
		{
			itemList.push_back(((*it)->name).c_str());
		}
	}
	return;
}


void EnemyLoot::Serialize(JSON_value* json) const
{
	assert(json != nullptr);
	std::string name;
	json->AddInt("numItems", items.size());
	for (int i = 0; i < items.size(); ++i) {
		name = "item" + std::to_string(i);
		json->AddString(name.c_str(),items[i].first->name.c_str());
		name = "drop" + std::to_string(i);
		json->AddInt(name.c_str(), items[i].second);
	}
}

void EnemyLoot::DeSerialize(JSON_value* json)
{
	assert(json != nullptr);
	std::string name;
	std::string pairf;
	int drop;
	int numItems = json->GetInt("numItems", items.size());
	for (int i = 0; i < numItems; ++i) {
		name = "item" + std::to_string(i);
		pairf = json->GetString(name.c_str());
		name = "drop" + std::to_string(i);
		drop = json->GetInt(name.c_str());
		actLoot.push_back(std::make_pair(pairf, drop));;
	}
}