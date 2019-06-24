#include "InventoryScript.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleInput.h"
#include "ModuleRender.h"
#include "ModuleUI.h"

#include "ComponentImage.h"
#include "ComponentTransform2D.h"
#include "ComponentAudioSource.h"

#include "PlayerMovement.h"

#include "GameObject.h"
#include "Viewport.h"


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

	menuPlayer = App->scene->FindGameObjectByName("BackgroundImage", inventory->parent)->GetComponent<Transform2D>();
	assert(menuPlayer != nullptr);

	selectItemAudio = (ComponentAudioSource*)App->scene->FindGameObjectByName("SelectItemAudio")->GetComponent<ComponentAudioSource>();
	assert(selectItemAudio != nullptr);
	dropItemAudio = (ComponentAudioSource*)App->scene->FindGameObjectByName("DropItemAudio")->GetComponent<ComponentAudioSource>();
	assert(dropItemAudio != nullptr);

	// Player
	GameObject* player = App->scene->FindGameObjectByName("Player");
	assert(player != nullptr);

	playerMovement = player->GetComponent<PlayerMovement>();
	assert(playerMovement != nullptr);
}

void InventoryScript::Update()
{
	if (!inventory->isActive()) return;

	math::float2 mouse = reinterpret_cast<const float2&>(App->input->GetMousePosition());
	float screenX = mouse.x - App->renderer->viewGame->winPos.x - (App->ui->currentWidth * .5f);
	float screenY = mouse.y - App->renderer->viewGame->winPos.y - (App->ui->currentHeight * .5f);

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
						playerMovement->Equip(pair.first.stats);
					} 
					else 
					{
						playerMovement->UnEquip(pair.first.stats);
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
