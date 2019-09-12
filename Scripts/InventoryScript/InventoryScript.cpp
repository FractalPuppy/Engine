#include "InventoryScript.h"
#include "JSON.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleInput.h"
#include "ModuleRender.h"
#include "ModuleUI.h"

#include "ComponentImage.h"
#include "ComponentTransform2D.h"
#include "ComponentAudioSource.h"

#include "GameObject.h"
#include "Viewport.h"
#include "PlayerPrefs.h"

#include "PlayerMovement.h"

InventoryScript_API Script* CreateScript()
{
	InventoryScript* instance = new InventoryScript;
	return instance;
}

void InventoryScript::Awake()
{

}

void InventoryScript::Start()
{
	inventory = App->scene->FindGameObjectByName("Inventory");
	GameObject* slots = App->scene->FindGameObjectByName("InventorySlots", inventory);
	assert(slots != nullptr);
	slotsTransform = slots->GetComponentsInChildren(ComponentType::Transform2D);

	slotsTransform.emplace_back(App->scene->FindGameObjectByName("HelmetSlot", inventory)->GetComponent<Transform2D>());
	slotsTransform.emplace_back(App->scene->FindGameObjectByName("BodySlot", inventory)->GetComponent<Transform2D>());
	slotsTransform.emplace_back(App->scene->FindGameObjectByName("PantsSlot", inventory)->GetComponent<Transform2D>());
	slotsTransform.emplace_back(App->scene->FindGameObjectByName("BootsSlot", inventory)->GetComponent<Transform2D>());
	slotsTransform.emplace_back(App->scene->FindGameObjectByName("WeaponSlot", inventory)->GetComponent<Transform2D>());
	slotsTransform.emplace_back(App->scene->FindGameObjectByName("AmuletSlot", inventory)->GetComponent<Transform2D>());

	std::list<GameObject*> list = App->scene->FindGameObjectByName("Items", inventory)->children;

	itemsSlots = { std::begin(list), std::end(list) }; //Pass gameObjects list to vector for better performance 

	GameObject* backgroundImage = App->scene->FindGameObjectByName("BackgroundImage", inventory->parent);
	if (backgroundImage != nullptr)
	{
		menuPlayer = backgroundImage->GetComponent<Transform2D>();
		assert(menuPlayer != nullptr);
	}
	else
	{
		LOG("Warning: Background Image couldn't be found.")
	}


	GameObject* selectAudioGO = App->scene->FindGameObjectByName("SelectItemAudio");
	if (selectAudioGO != nullptr)
	{
		selectItemAudio = (ComponentAudioSource*)selectAudioGO->GetComponent<ComponentAudioSource>();
		assert(selectItemAudio != nullptr);
	}
	else
	{
		LOG("Warning: Select Item Audio couldn't be found.")
	}

	GameObject* dropAudioGO = App->scene->FindGameObjectByName("DropItemAudio");
	if (dropAudioGO != nullptr)
	{
		dropItemAudio = (ComponentAudioSource*)dropAudioGO->GetComponent<ComponentAudioSource>();
		assert(dropItemAudio != nullptr);
	}
	else
	{
		LOG("Warning: Drop Item Audio couldn't be found.")
	}

	// Player
	GameObject* player = App->scene->FindGameObjectByName("Player");
	assert(player != nullptr);
	if (player != nullptr)
	{
		playerMovement = player->GetComponent<PlayerMovement>();
		assert(playerMovement != nullptr);
	}

	LoadInventory();
}

void InventoryScript::Update()
{
	if (!inventory->isActive()) return;

	math::float2 mouse = reinterpret_cast<const float2&>(App->input->GetMousePosition());
	float screenX, screenY;

	if (App->renderer->viewGame != nullptr)
	{
		screenX = mouse.x - App->renderer->viewGame->winPos.x - (App->ui->currentWidth * .5f);
		screenY = mouse.y - App->renderer->viewGame->winPos.y - (App->ui->currentHeight * .5f);
	}
	else
	{
		screenX = mouse.x - (App->ui->currentWidth * .5f);
		screenY = mouse.y - (App->ui->currentHeight * .5f);
	}

	for (int i = 0; i < itemsSlots.size(); ++i)
	{
		if (!itemsSlots[i]->isActive()) continue;

		Transform2D* rectTransform = itemsSlots[i]->GetComponent<Transform2D>();
		ComponentImage* image = itemsSlots[i]->GetComponent<ComponentImage>();

		if (image->isHovered && App->input->GetMouseButtonDown(1) == KEY_DOWN)
		{
			image->isPressed = true;
			initialitemPos = rectTransform->getPosition();
			if (!itemGrabbed) selectItemAudio->Play();
			itemGrabbed = true;
		}


		if (image->isPressed)
		{
			math::float2 newPos = math::float2(screenX, -screenY);
			rectTransform->SetPositionUsingAligment(newPos);
		}

		if (image->isPressed && App->input->GetMouseButtonDown(1) == KEY_UP)
		{
			bool moved = false;
			image->isPressed = false;
			itemGrabbed = false; //Audio

			math::float2 menuPos = menuPlayer->getPosition();
			math::float2 menuSize = menuPlayer->getSize();
			math::float2 menuMin = float2(menuPos.x - menuSize.x *.5f, -menuPos.y - menuSize.y *.5f);
			math::float2 menuMax = float2(menuPos.x + menuSize.x *.5f, -menuPos.y + menuSize.y *.5f);
			if (!(screenX > menuMin.x && screenX < menuMax.x && screenY > menuMin.y && screenY < menuMax.y))
			{
				rectTransform->SetPositionUsingAligment(initialitemPos);
				itemsSlots[i]->SetActive(false);
				for (int j = 0; j < items.size(); ++j)
				{
					if (items[j].second == i)
					{
						items.erase(items.begin() + j);
						return;
					}
				}
				return;
			}

			std::pair<Item, int> pair;
			for (int t = 0; t < items.size(); ++t)
			{
				if (items[t].second == i)
				{
					pair = items[t];
					break;
				}
			}

			for (int j = 0; j < TOTAL_SLOTS; ++j)
			{
				math::float2 posSlot = ((Transform2D*)slotsTransform[j])->getPosition();
				math::float2 sizeSlot = ((Transform2D*)slotsTransform[j])->getSize();
				math::float2 slotMin = float2(posSlot.x - sizeSlot.x *.5f, -posSlot.y - sizeSlot.y *.5f);
				math::float2 slotMax = float2(posSlot.x + sizeSlot.x *.5f, -posSlot.y + sizeSlot.y *.5f);
				if (screenX > slotMin.x && screenX < slotMax.x && screenY > slotMin.y && screenY < slotMax.y)
				{
					if (j >= INVENTARY_SLOTS)
					{
						bool exit = false;
						switch (j)
						{
						case 18:
							if (pair.first.type != ItemType::HELMET)
							{
								exit = true;
							}
							else
							{
								pair.first.isEquipped = !pair.first.isEquipped;
							}
							break;
						case 19:
							if (pair.first.type != ItemType::CHEST)
							{
								exit = true;
							}
							else
							{
								pair.first.isEquipped = !pair.first.isEquipped;
							}
							break;
						case 20:
							if (pair.first.type != ItemType::PANTS)
							{
								exit = true;
							}
							else
							{
								pair.first.isEquipped = !pair.first.isEquipped;
							}
							break;
						case 21:
							if (pair.first.type != ItemType::BOOTS)
							{
								exit = true;
							}
							else
							{
								pair.first.isEquipped = !pair.first.isEquipped;
							}
							break;
						case 22:
							if (pair.first.type != ItemType::WEAPON)
							{ 
								exit = true;
							}
							else
							{
								pair.first.isEquipped = !pair.first.isEquipped;
							}
							break;
						case 23:
							if (pair.first.type != ItemType::AMULET)
							{
								exit = true;
							}
							else
							{
								pair.first.isEquipped = !pair.first.isEquipped;
							}
							break;
						default:
							break;
						}

						if (exit) break;
					}

					moved = true;
					math::float2 newPos = math::float2(posSlot.x, posSlot.y);
					rectTransform->SetPositionUsingAligment(newPos);

					if (itemsSlots[j]->UUID != itemsSlots[i]->UUID)
					{

						itemsSlots[j]->GetComponent<Transform2D>()->SetPositionUsingAligment(initialitemPos);
						GameObject* tmp(std::move(itemsSlots[j]));
						itemsSlots[j] = std::move(itemsSlots[i]);
						itemsSlots[i] = std::move(tmp);
						tmp = nullptr;

						dropItemAudio->Play();

						for (int z = 0; z < items.size(); ++z)
						{
							if (items[z].second == i)
							{
								items[z].second = j;			
								continue;
							} 
							if (items[z].second == j)
							{
								items[z].second = i;
								continue;
							}
						}
					}

					if (pair.first.isEquipped)
					{
						for (std::vector<std::pair<Item, int>>::iterator it = items.begin(); it != items.end(); it++)
						{
							if (it._Ptr->first.type == pair.first.type && it._Ptr->first.isEquipped)
							{
								playerMovement->UnEquip(it._Ptr->first.stats);
								it._Ptr->first.isEquipped = false;
							}
							if (it._Ptr->first.name == pair.first.name)
							{
								it._Ptr->first.isEquipped = true;
								playerMovement->Equip(pair.first.stats);
								break;
							}
						}						
					} 
					else 
					{
						for (std::vector<std::pair<Item, int>>::iterator it = items.begin(); it != items.end(); it++)
						{
							if (it._Ptr->first.name == pair.first.name && it._Ptr->first.isEquipped)
							{
								it._Ptr->first.isEquipped = false;
								playerMovement->UnEquip(pair.first.stats);
								break;
							}
						}
					}

					break;
				}
			}

			if (!moved)
			{
				rectTransform->SetPositionUsingAligment(initialitemPos);
				initialitemPos = math::float2::zero;
				return;
			}

		}
	}
}

bool InventoryScript::AddItem(Item item)
{

	for (int i = 0; i < INVENTARY_SLOTS; ++i)
	{
		if (!itemsSlots[i]->activeSelf)
		{
			itemsSlots[i]->SetActive(true);
			ComponentImage* image = itemsSlots[i]->GetComponent<ComponentImage>();
			image->UpdateTexture(item.sprite);
			items.emplace_back(std::make_pair(item, i));
			return true;
		}
	}
	return false;
}

std::vector<Item> InventoryScript::GetQuickItems()
{
	std::vector<Item> itemsToReturn;
	for (int i = 0; i < items.size(); ++i)
	{
		if (items[i].first.type == ItemType::QUICK)
		{
			itemsToReturn.emplace_back(items[i].first);
		}
	}

	return itemsToReturn;
}


void InventoryScript::SaveInventory()
{
	JSON* inventory = new JSON();
	JSON_value *itemsJSON = inventory->CreateValue(rapidjson::kArrayType);
	for (int i = 0; i < items.size(); ++i)
	{
		JSON_value *item = itemsJSON->CreateValue();
		item->AddInt("type", (int)items[i].first.type);
		item->AddString("name", items[i].first.name.c_str());
		item->AddString("description", items[i].first.description.c_str());
		item->AddString("sprite", items[i].first.sprite.c_str());
		item->AddInt("equiped", items[i].first.isEquipped);
		item->AddFloat("dexterity", items[i].first.stats.dexterity);
		item->AddFloat("health", items[i].first.stats.health);
		item->AddFloat("hpRegen", items[i].first.stats.hpRegen);
		item->AddFloat("mana", items[i].first.stats.mana);
		item->AddFloat("manaRegen", items[i].first.stats.manaRegen);
		item->AddFloat("strength", items[i].first.stats.strength);
		item->AddInt("position", items[i].second);
		itemsJSON->AddValue("", *item);
	}
	inventory->AddValue("items", *itemsJSON);
	PlayerPrefs::SaveJson(inventory, "Inventory");
}

void InventoryScript::LoadInventory()
{
	JSON* inventory = PlayerPrefs::LoadJson("Inventory");
	if (inventory != nullptr) {
		JSON_value* itemsJSON = inventory->GetValue("items");
		items.clear();
		for (unsigned i = 0; i < itemsJSON->Size(); i++)
		{
			JSON_value* itemJSON = itemsJSON->GetValue(i);
			int position = itemJSON->GetInt("position");
			Item item;
			item.type = (ItemType)itemJSON->GetInt("type");
			item.name = itemJSON->GetString("name");
			item.description = itemJSON->GetString("description");
			item.sprite = itemJSON->GetString("sprite");
			item.isEquipped = itemJSON->GetInt("equiped");
			item.stats.dexterity = itemJSON->GetFloat("dexterity");
			item.stats.health = itemJSON->GetFloat("health");
			item.stats.hpRegen = itemJSON->GetFloat("hpRegen");
			item.stats.mana = itemJSON->GetFloat("mana");
			item.stats.manaRegen = itemJSON->GetFloat("manaRegen");
			item.stats.strength = itemJSON->GetFloat("strength");

			itemsSlots[position]->SetActive(true);
			ComponentImage* image = itemsSlots[position]->GetComponent<ComponentImage>();
			image->UpdateTexture(item.sprite);
			items.emplace_back(std::make_pair(item, position));
		}
	}
}
