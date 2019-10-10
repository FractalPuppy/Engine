#include "EquipPopupController.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleInput.h"
#include "ModuleResourceManager.h"

#include "GameObject.h"
#include "ComponentTransform2D.h"
#include "ComponentButton.h"
#include "ComponentText.h"
#include "ComponentImage.h"
#include "ResourceTexture.h"

#include "PlayerMovement.h"
#include "InventoryScript.h"
#include "SkillTreeController.h"
#include "Item.h"
#include "Skill.h"

#include "JSON.h"
#include "PlayerPrefs.h"
#include "Math/float2.h"

EquipPopupController_API Script* CreateScript()
{
	EquipPopupController* instance = new EquipPopupController;
	return instance;
}

void EquipPopupController::Start()
{
	inventory = App->scene->FindGameObjectByName("Inventory")->GetComponent<InventoryScript>();
	assert(inventory != nullptr);

	skillTree = App->scene->FindGameObjectByName("Skills")->GetComponent<SkillTreeController>();
	assert(inventory != nullptr);


	popupGOSkills = App->scene->FindGameObjectByName("PopUpEquipSkills", gameobject);
	popupGOItems = App->scene->FindGameObjectByName("PopUpEquipItems", gameobject);

	backgroundSkills = popupGOSkills->GetComponent<ComponentImage>();
	assert(backgroundSkills != nullptr);
	backgroundItems = popupGOItems->GetComponent<ComponentImage>();
	assert(backgroundItems != nullptr);

	items = App->scene->FindGameObjectByName("PopUpItems", gameobject)->GetComponent<Button>();
	assert(items != nullptr);

	skills = App->scene->FindGameObjectByName("PopUpSkills", gameobject)->GetComponent<Button>();
	assert(skills != nullptr);

	unequip = App->scene->FindGameObjectByName("PopUpUnequip", gameobject)->GetComponent<Button>();
	assert(unequip != nullptr);

	GameObject* HUD = App->scene->FindGameObjectByName("GameHUB");
	assert(HUD != nullptr);
	hudButtons.emplace_back(App->scene->FindGameObjectByName("RC", HUD)->GetComponent<Button>());
	hudButtons.emplace_back(App->scene->FindGameObjectByName("One", HUD)->GetComponent<Button>());
	hudButtons.emplace_back(App->scene->FindGameObjectByName("Two", HUD)->GetComponent<Button>());
	hudButtons.emplace_back(App->scene->FindGameObjectByName("Three", HUD)->GetComponent<Button>());
	hudButtons.emplace_back(App->scene->FindGameObjectByName("For", HUD)->GetComponent<Button>());
	hudButtons.emplace_back(App->scene->FindGameObjectByName("Q", HUD)->GetComponent<Button>());
	hudButtons.emplace_back(App->scene->FindGameObjectByName("W", HUD)->GetComponent<Button>());
	hudButtons.emplace_back(App->scene->FindGameObjectByName("E", HUD)->GetComponent<Button>());
	hudButtons.emplace_back(App->scene->FindGameObjectByName("R", HUD)->GetComponent<Button>());
	hudImageSlots.emplace_back(App->scene->FindGameObjectByName("RC_Slot", HUD)->GetComponent<ComponentImage>());
	hudImageSlots.emplace_back(App->scene->FindGameObjectByName("One_Slot", HUD)->GetComponent<ComponentImage>());
	hudImageSlots.emplace_back(App->scene->FindGameObjectByName("Two_Slot", HUD)->GetComponent<ComponentImage>());
	hudImageSlots.emplace_back(App->scene->FindGameObjectByName("Three_Slot", HUD)->GetComponent<ComponentImage>());
	hudImageSlots.emplace_back(App->scene->FindGameObjectByName("Four_Slot", HUD)->GetComponent<ComponentImage>());
	hudImageSlots.emplace_back(App->scene->FindGameObjectByName("Q_Slot", HUD)->GetComponent<ComponentImage>());
	hudImageSlots.emplace_back(App->scene->FindGameObjectByName("W_Slot", HUD)->GetComponent<ComponentImage>());
	hudImageSlots.emplace_back(App->scene->FindGameObjectByName("E_Slot", HUD)->GetComponent<ComponentImage>());
	hudImageSlots.emplace_back(App->scene->FindGameObjectByName("R_Slot", HUD)->GetComponent<ComponentImage>());
	hudButtonsText.emplace_back(App->scene->FindGameObjectByName("RCText", HUD)->GetComponent<Transform2D>(), false);
	hudButtonsText.emplace_back(App->scene->FindGameObjectByName("OneText", HUD)->GetComponent<Transform2D>(), false);
	hudButtonsText.emplace_back(App->scene->FindGameObjectByName("TwoText", HUD)->GetComponent<Transform2D>(), false);
	hudButtonsText.emplace_back(App->scene->FindGameObjectByName("ThreeText", HUD)->GetComponent<Transform2D>(), false);
	hudButtonsText.emplace_back(App->scene->FindGameObjectByName("FourText", HUD)->GetComponent<Transform2D>(), false);
	hudButtonsText.emplace_back(App->scene->FindGameObjectByName("QText", HUD)->GetComponent<Transform2D>(), false);
	hudButtonsText.emplace_back(App->scene->FindGameObjectByName("WText", HUD)->GetComponent<Transform2D>(), false);
	hudButtonsText.emplace_back(App->scene->FindGameObjectByName("EText", HUD)->GetComponent<Transform2D>(), false);
	hudButtonsText.emplace_back(App->scene->FindGameObjectByName("RText", HUD)->GetComponent<Transform2D>(), false);
	hudConsumibleItemsQuantity.emplace_back(App->scene->FindGameObjectByName("RC_Number", HUD)->GetComponent<Text>());
	hudConsumibleItemsQuantity.emplace_back(App->scene->FindGameObjectByName("One_Number", HUD)->GetComponent<Text>());
	hudConsumibleItemsQuantity.emplace_back(App->scene->FindGameObjectByName("Two_Number", HUD)->GetComponent<Text>());
	hudConsumibleItemsQuantity.emplace_back(App->scene->FindGameObjectByName("Three_Number", HUD)->GetComponent<Text>());
	hudConsumibleItemsQuantity.emplace_back(App->scene->FindGameObjectByName("Four_Number", HUD)->GetComponent<Text>());
	hudConsumibleItemsQuantity.emplace_back(App->scene->FindGameObjectByName("Q_Number", HUD)->GetComponent<Text>());
	hudConsumibleItemsQuantity.emplace_back(App->scene->FindGameObjectByName("W_Number", HUD)->GetComponent<Text>());
	hudConsumibleItemsQuantity.emplace_back(App->scene->FindGameObjectByName("E_Number", HUD)->GetComponent<Text>());
	hudConsumibleItemsQuantity.emplace_back(App->scene->FindGameObjectByName("R_Number", HUD)->GetComponent<Text>());

	std::list<GameObject*> listSkills = App->scene->FindGameObjectByName("PopUpSlotsSkills", popupGOSkills)->children;
	std::list<GameObject*> listItems = App->scene->FindGameObjectByName("PopUpSlotsItems", popupGOItems)->children;

	slotsSkills = { std::begin(listSkills), std::end(listSkills) };
	slotsItems = { std::begin(listItems), std::end(listItems) };

	hoverTransform = App->scene->FindGameObjectByName("PopUpHover", gameobject)->GetComponent<Transform2D>();
	assert(hoverTransform != nullptr);

	GameObject* playerGO = App->scene->FindGameObjectByName("Player");
	assert(playerGO != nullptr);
	if (playerGO != nullptr)
	{
		player = playerGO->GetComponent<PlayerMovement>();
		assert(player != nullptr);
	}

	LoadPopUp();
}

void EquipPopupController::Update()
{
	int positionClicked = inventory->ConsumeItemsController();
	for (int i : inventory->equipedConsumablesToRemove) RemoveEquipedConsumable(i);
	inventory->equipedConsumablesToRemove.clear();

	if (positionClicked != -1)
	{
		for (int i = 0; i < itemsEquiped.size(); ++i)
		{
			if (positionClicked == itemsEquiped[i].first)
			{
				hudConsumibleItemsQuantity[positionClicked]->text = std::to_string(inventory->GetCurrentQuantity(itemsEquiped[i].second));
			}
		}
	}

	//Check if has to close PopUp
	if (gameobject->isActive() && !backgroundSkills->isHovered && !backgroundItems->isHovered && !items->IsHovered() && !skills->IsHovered() && (App->input->GetMouseButtonDown(1) == KEY_UP || App->input->GetMouseButtonDown(3) == KEY_UP))
	{
		gameobject->SetActive(false);
		return;
	}

	//Check if has to open PopUp
	for (int i = 0; i < HUD_BUTTONS; ++i)
	{
		if (hudButtons[i]->IsHovered() && (App->input->GetMouseButtonDown(3) == KEY_UP || App->input->GetMouseButtonDown(1) == KEY_UP)) {
			activeButton = i;
			if (!gameobject->isActive())
			{
				gameobject->SetActive(true);
				FillLists();
			}
			break;
		}
	}

	if (!gameobject->isActive()) { return; }

	hoverTransform->gameobject->SetActive(false);

	//Update Popup info
	ChangePopUpSlots();


	if (unequip->KeyUp())
	{
		CleanButton();
	}

	//Check if selected
	if (popupGOSkills->isActive())
	{
		for (int i = 0; i < POPUP_SKILLS_SLOTS; ++i)
		{
			if (!slotsSkills[i]->isActive())
			{
				break;
			}
			if (slotsSkills[i]->GetComponent<ComponentImage>()->isHovered)
			{
				math::float2 pos = slotsSkills[i]->GetComponent<Transform2D>()->getPosition();
				hoverTransform->SetPositionUsingAligment(pos);
				hoverTransform->gameobject->SetActive(true);

				//TODO: Show info
				if (App->input->GetMouseButtonDown(1) == KEY_UP)
				{
					slotsSkills[i]->GetComponent<ComponentImage>()->isHovered = false;
					Assign(i);
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < POPUP_ITEMS_SLOTS; ++i)
		{
			if (!slotsItems[i]->isActive())
			{
				break;
			}
			if (slotsItems[i]->GetComponent<ComponentImage>()->isHovered)
			{
				math::float2 pos = slotsItems[i]->GetComponent<Transform2D>()->getPosition();
				hoverTransform->SetPositionUsingAligment(pos);
				hoverTransform->gameobject->SetActive(true);

				//TODO: Show info
				if (App->input->GetMouseButtonDown(1) == KEY_UP)
				{
					slotsItems[i]->GetComponent<ComponentImage>()->isHovered = false;
					Assign(i);
				}
			}
		}
	}
}

void EquipPopupController::Assign(int i)
{
	if (popupGOSkills->isActive())
	{
		RemoveEquiped();
		skillsEquiped.emplace_back(activeButton, skillsList[i]);
		hudImageSlots[activeButton]->UpdateTexture(skillsList[i].spriteActive->GetName());
		hudImageSlots[activeButton]->gameobject->SetActive(true);
		hudConsumibleItemsQuantity[activeButton]->gameobject->SetActive(false);
		MoveNumber(activeButton);
		player->AssignSkill((SkillType)skillsList[i].id, activeButton);
	}
	else
	{
		RemoveEquiped();
		itemsEquiped.emplace_back(activeButton, itemsList[i]);
		hudImageSlots[activeButton]->UpdateTexture(itemsList[i].sprite);
		hudImageSlots[activeButton]->gameobject->SetActive(true);
		hudConsumibleItemsQuantity[activeButton]->gameobject->SetActive(true);
		hudConsumibleItemsQuantity[activeButton]->uiOrder = 11;
		hudConsumibleItemsQuantity[activeButton]->text = std::to_string(inventory->GetCurrentQuantity(itemsList[i]));
		MoveNumber(activeButton);
		inventory->AssignConsumableItem(itemsList[i], activeButton);
	}
	gameobject->SetActive(false);
}

void EquipPopupController::MoveNumber(int number)
{
	if (!hudButtonsText[number].second)
	{
		math::float2 newPos = hudButtonsText[number].first->getPosition();
		newPos.y -= 15.7;
		hudButtonsText[number].first->SetPositionUsingAligment(newPos);
		hudButtonsText[number].second = true;
	}
}

void EquipPopupController::RemoveEquiped()
{
	for (int j = 0; j < skillsEquiped.size(); ++j)
	{
		if (skillsEquiped[j].first == activeButton)
		{
			skillsEquiped.erase(skillsEquiped.begin() + j);
		}
	}
	for (int j = 0; j < itemsEquiped.size(); ++j)
	{
		if (itemsEquiped[j].first == activeButton)
		{
			itemsEquiped.erase(itemsEquiped.begin() + j);
			inventory->UnAssignConsumableItem(activeButton);
		}
	}
}

void EquipPopupController::ChangePopUpSlots()
{
	if (items->KeyUp() && popupGOSkills->isActive())
	{
		popupGOItems->SetActive(true);
		popupGOSkills->SetActive(false);
		FillItemSlots();
	}

	if (skills->KeyUp() && !popupGOSkills->isActive())
	{
		popupGOItems->SetActive(false);
		popupGOSkills->SetActive(true);
		FillSkillSlots();
	}
}


void EquipPopupController::FillLists()
{

	skillsList.clear();
	itemsList.clear();
	skillsList = skillTree->GetActiveSkills();
	itemsList = inventory->GetQuickItems();

	if (popupGOSkills->isActive())
	{
		FillSkillSlots();
	}
	else
	{
		FillItemSlots();
	}
}

void EquipPopupController::FillSkillSlots()
{

	for (int i = 0; i < POPUP_SKILLS_SLOTS; ++i)
	{
		slotsSkills[i]->SetActive(false);
		if (i < skillsList.size())
		{
			slotsSkills[i]->GetComponent<ComponentImage>()->UpdateTexture(skillsList[i].spriteActive->GetName());
			slotsSkills[i]->SetActive(true);
		}
	}
}

void EquipPopupController::FillItemSlots()
{

	if (popupGOSkills->isActive()) {
		return;
	}

	for (int i = 0; i < POPUP_ITEMS_SLOTS; ++i)
	{
		slotsItems[i]->SetActive(false);

		if (i < itemsList.size())
		{
			slotsItems[i]->GetComponent<ComponentImage>()->UpdateTexture(itemsList[i].sprite);
			slotsItems[i]->SetActive(true);
		}

	}
}

void EquipPopupController::CleanButton()
{
	player->AssignSkill(SkillType::NONE, activeButton);

	RemoveEquiped();

	hudImageSlots[activeButton]->gameobject->SetActive(false);

	if (hudButtonsText[activeButton].second)
	{
		math::float2 newPos = hudButtonsText[activeButton].first->getPosition();
		newPos.y += 15.7;
		hudButtonsText[activeButton].first->SetPositionUsingAligment(newPos);
		hudButtonsText[activeButton].second = false;
		hudConsumibleItemsQuantity[activeButton]->gameobject->SetActive(false);
		hudConsumibleItemsQuantity[activeButton]->text = std::to_string(1);
	}
}

void EquipPopupController::SavePopUp()
{
	JSON* popup = new JSON();
	JSON_value *skillsJSON = popup->CreateValue(rapidjson::kArrayType);
	for (int i = 0; i < skillsEquiped.size(); ++i)
	{
		JSON_value *skill = skillsJSON->CreateValue();
		skill->AddInt("id", (int)skillsEquiped[i].second.id);
		skill->AddString("name", skillsEquiped[i].second.name.c_str());
		skill->AddString("description", skillsEquiped[i].second.description.c_str());
		skill->AddString("sprite", skillsEquiped[i].second.spriteActive->GetName());
		skill->AddInt("currentLevel", skillsEquiped[i].second.currentLevel);
		skill->AddInt("position", skillsEquiped[i].first);
		skillsJSON->AddValue("", *skill);
	}
	popup->AddValue("skills", *skillsJSON);

	JSON_value *itemsJSON = popup->CreateValue(rapidjson::kArrayType);
	for (int i = 0; i < itemsEquiped.size(); ++i)
	{
		JSON_value *item = itemsJSON->CreateValue();
		item->AddInt("type", (int)itemsEquiped[i].second.type);
		item->AddString("name", itemsEquiped[i].second.name.c_str());
		item->AddString("description", itemsEquiped[i].second.description.c_str());
		item->AddString("sprite", itemsEquiped[i].second.sprite.c_str());
		item->AddInt("equiped", itemsEquiped[i].second.isEquipped);
		item->AddFloat("dexterity", itemsEquiped[i].second.stats.dexterity);
		item->AddFloat("health", itemsEquiped[i].second.stats.health);
		item->AddFloat("hpRegen", itemsEquiped[i].second.stats.hpRegen);
		item->AddFloat("mana", itemsEquiped[i].second.stats.mana);
		item->AddFloat("manaRegen", itemsEquiped[i].second.stats.manaRegen);
		item->AddFloat("strength", itemsEquiped[i].second.stats.strength);
		item->AddInt("position", itemsEquiped[i].first);
		itemsJSON->AddValue("", *item);
	}
	popup->AddValue("items", *itemsJSON);
	PlayerPrefs::SaveJson(popup, "PopUp");
}

void EquipPopupController::RemoveEquipedConsumable(int assignedButton)
{

	player->AssignSkill(SkillType::NONE, assignedButton);

	for (int j = 0; j < itemsEquiped.size(); ++j)
	{
		if (itemsEquiped[j].first == assignedButton)
		{
			itemsEquiped.erase(itemsEquiped.begin() + j);
			inventory->UnAssignConsumableItem(assignedButton);
		}
	}

	hudImageSlots[assignedButton]->gameobject->SetActive(false);

	if (hudButtonsText[assignedButton].second)
	{
		math::float2 newPos = hudButtonsText[assignedButton].first->getPosition();
		newPos.y += 15.7;
		hudButtonsText[assignedButton].first->SetPositionUsingAligment(newPos);
		hudButtonsText[assignedButton].second = false;
		hudConsumibleItemsQuantity[assignedButton]->gameobject->SetActive(false);
		hudConsumibleItemsQuantity[assignedButton]->text = std::to_string(1);
	}
}

void EquipPopupController::LoadPopUp()
{
	JSON* popup = PlayerPrefs::LoadJson("PopUp");
	if (popup != nullptr) {
		JSON_value* itemsJSON = popup->GetValue("items");
		itemsEquiped.clear();
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

			itemsEquiped.emplace_back(position, item);
			hudImageSlots[position]->UpdateTexture(item.sprite);
			hudImageSlots[position]->gameobject->SetActive(true);
			MoveNumber(position);
		}

		JSON_value* skillsJSON = popup->GetValue("skills");
		skillsEquiped.clear();
		for (unsigned i = 0; i < skillsJSON->Size(); i++)
		{
			JSON_value* skillJSON = skillsJSON->GetValue(i);
			int position = skillJSON->GetInt("position");
			Skill skill;
			skill.id = skillJSON->GetInt("id");
			skill.name = skillJSON->GetString("name");
			skill.description = skillJSON->GetString("description");
			skill.spriteActive = (ResourceTexture*)App->resManager->GetByName(skillJSON->GetString("sprite"), TYPE::TEXTURE);
			skill.currentLevel = skillJSON->GetInt("currentLevel");

			skillsEquiped.emplace_back(position, skill);
			hudImageSlots[position]->UpdateTexture(skill.spriteActive->GetName());
			hudImageSlots[position]->gameobject->SetActive(true);
			MoveNumber(position);
		}

	}
}
