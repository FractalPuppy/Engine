#include "ItemPicker.h"
#include "Application.h"
#include "ModuleScene.h"
#include "ModuleInput.h"
#include "ModuleResourceManager.h"
#include "ModuleTime.h"
#include "ModuleUI.h"

#include "GameObject.h"
#include "ComponentRenderer.h"
#include "ComponentCamera.h"
#include "MouseController.h"

#include "InventoryScript.h"
#include "ItemNameController.h"

#include "JSON.h"
#include "imgui.h"
#include "Resource.h"
#include "ResourceMesh.h"
#include "ResourceMaterial.h"
#include "ComponentAudioSource.h"
#include "HashString.h"
#include <algorithm>

#define None "None Selected"

#pragma warning(disable : 4996)

ItemPicker_API Script* CreateScript()
{
	ItemPicker* instance = new ItemPicker;
	return instance;
}

void ItemPicker::Expose(ImGuiContext* context)
{
	ImGui::Separator();
	ImGui::Text("Item cursor:");
	char* itemCursorAux = new char[64];
	strcpy_s(itemCursorAux, strlen(itemCursor.c_str()) + 1, itemCursor.c_str());
	ImGui::InputText("itemCursor", itemCursorAux, 64);
	itemCursor = itemCursorAux;
	delete[] itemCursorAux;
	ImGui::Separator();

	ImGui::SetCurrentContext(context);
	const char* types[] = { "NONE","QUICK","KEY","MATERIAL","WEAPON","HELMET","CHEST","PANTS","BOOTS","AMULET","RING" };
	const char* rarities[] = { "BASIC","RARE","EPIC","LEGENDARY" };
	if (ImGui::BeginCombo("Type", types[(int)type]))
	{
		for (int n = 0; n < 11; n++)
		{
			bool isSelected = ((int)type == n);
			if (ImGui::Selectable(types[n], isSelected) && (int)type != n)
			{
				type = (ItemType)n;
			}
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	if (ImGui::BeginCombo("Rarity", rarities[(int)rarity]))
	{
		for (int n = 0; n < 4; n++)
		{
			bool isSelected = ((int)rarity == n);
			if (ImGui::Selectable(rarities[n], isSelected) && (int)rarity != n)
			{
				rarity = (ItemRarity)n;
			}
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	if (textureFiles.empty())
	{
		textureFiles = App->resManager->GetResourceNamesList(TYPE::TEXTURE, true);
	}

	//texture selector
	if (ImGui::BeginCombo("Texture", sprite.c_str()))
	{
		bool none_selected = (sprite == "None");
		if (ImGui::Selectable("None", none_selected))
		{
			sprite = "None";
		}
		if (none_selected)
			ImGui::SetItemDefaultFocus();
		for (int n = 0; n < textureFiles.size(); n++)
		{
			bool is_selected = (sprite == textureFiles[n]);
			if (ImGui::Selectable(textureFiles[n].c_str(), is_selected) && !is_selected)
			{
				sprite = textureFiles[n].c_str();
			}
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	char* imguiText = new char[64];
	strcpy(imguiText, name.c_str());
	ImGui::InputText("##", imguiText, 64);
	name = imguiText;
	delete[] imguiText;

	char* bboxName = new char[64];
	strcpy_s(bboxName, strlen(myBboxName.c_str()) + 1, myBboxName.c_str());
	ImGui::InputText("My BBox Name", bboxName, 64);
	myBboxName = bboxName;
	delete[] bboxName;

	// Mesh selector
	ImGui::Text("Mesh");
	ImGui::PushID("Mesh Combo");
	if (ImGui::BeginCombo("", itemMesh != nullptr ? itemMesh->GetName() : "None selected"))
	{
		if (meshesList.empty())
		{
			meshesList.clear();
			meshesList = App->resManager->GetResourceNamesList(TYPE::MESH, true);
		}
		bool none_selected = (itemMesh == nullptr);
		if (ImGui::Selectable(None, none_selected))
		{
			item.meshUID = 0u;
			itemMesh = nullptr;
		}
		if (none_selected)
			ImGui::SetItemDefaultFocus();

		for (int n = 0; n < meshesList.size(); n++)
		{
			bool is_selected = (itemMesh != nullptr ? itemMesh->GetName() == meshesList[n].c_str() : false);
			if (ImGui::Selectable(meshesList[n].c_str(), is_selected))
			{
				if (itemMesh == nullptr || itemMesh->GetName() != meshesList[n])
				{
					unsigned meshUID = App->resManager->FindByName(meshesList[n].c_str(), TYPE::MESH);
					if (meshUID != 0u)
					{
						item.meshUID = meshUID;
						itemMesh = (ResourceMesh*)App->resManager->GetWithoutLoad(meshUID);
					}
				}
			}
			if (is_selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	else
	{
		meshesList.clear();
	}

	// Material selector
	ImGui::Text("Material");
	ImGui::PushID("Material Combo");
	if (ImGui::BeginCombo("", itemMaterial != nullptr ? itemMaterial->GetName() : "None selected"))
	{
		if (materialsList.empty())
		{
			materialsList.clear();
			materialsList = App->resManager->GetResourceNamesList(TYPE::MATERIAL, true);
		}
		for (int n = 0; n < materialsList.size(); n++)
		{
			bool is_selected = (itemMaterial != nullptr ? itemMaterial->GetName() == materialsList[n].c_str() : false);
			if (ImGui::Selectable(materialsList[n].c_str(), is_selected))
			{
				if (itemMaterial == nullptr || itemMaterial->GetName() != materialsList[n])
				{
					unsigned materialUID = App->resManager->FindByName(materialsList[n].c_str(), TYPE::MATERIAL);
					if (materialUID != 0u)
					{
						item.materialUID = materialUID;
						itemMaterial = (ResourceMaterial*)App->resManager->GetWithoutLoad(materialUID);
					}
				}
			}
			if (is_selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	else
	{
		materialsList.clear();
	}

	item.stats.Expose("Item Stats");
}

void ItemPicker::Start()
{
	inventoryScript = App->scene->FindGameObjectByName("Inventory")->GetComponent<InventoryScript>();
	GameObject* playerGO = App->scene->FindGameObjectByName("Player");

	//now we store the playermovement script to tell it when item clicked and stuff
	playerMovementScript = (PlayerMovement*)playerGO->GetComponentOld(ComponentType::Script);

	playerBbox = &App->scene->FindGameObjectByName("PlayerMesh", playerGO)->bbox;

	myBboxName = gameobject->name;

	item.name = this->name;
	item.description = this->description;
	item.sprite = this->sprite;
	item.type = this->type;

	GameObject* GOtemp = App->scene->FindGameObjectByName("itemPickedAudio");
	if (GOtemp != nullptr)
	{
		itemPickedAudio = GOtemp->GetComponent<ComponentAudioSource>();
		assert(itemPickedAudio != nullptr);
	}
	else
	{
		LOG("The Game Object 'itemPickedAudio' couldn't be found.");
	}

	GameObject* itemNameGO = App->scene->FindGameObjectByName("ItemName");
	if (itemNameGO == nullptr)
	{
		LOG("Item name controller GO couldn't be found \n");
	}
	else
	{
		itemName = itemNameGO->GetComponent<ItemNameController>();
		if (itemName != nullptr)
		{
			LOG("Item name controller couldn't be found \n");
		}
	}
}

void ItemPicker::PickupItem() const
{
	if (inventoryScript->AddItem(item))
	{
		gameobject->SetActive(false);
		if (itemPickedAudio != nullptr) itemPickedAudio->Play();
		itemName->DisableName(gameobject->UUID);
	}

	MouseController::ChangeCursorIcon(gameStandarCursor);
	App->ui->SetIsItemHover(false);
}

void ItemPicker::Update()
{
	if (!gameobject->isActive())
	{
		if (std::find(nameShowed.begin(), nameShowed.end(), gameobject->UUID) != nameShowed.end())
		{
			nameShowed.erase(std::find(nameShowed.begin(), nameShowed.end(), gameobject->UUID));
			itemName->DisableName(gameobject->UUID);
		}
		return;
	}
	//checking if gotta pickup item
	math::float3 closestPoint;
	//first check if item clicked (either the item mesh or its name)
	if ((App->scene->Intersects(closestPoint, myBboxName.c_str()) || itemName->Intersection(gameobject->UUID)) &&
		App->input->GetMouseButtonDown(1) == KEY_DOWN)
	{
		//if player next to the item
		if (gameobject->bbox.Intersects(*playerBbox))
		{
				PickupItem();
		}
		//if not, player goes towards it
		else
		{
			playerMovementScript->stoppedGoingToItem = false;
			playerMovementScript->itemClicked = this;
		}
	}
	//check if the player clicked outside of the item (if it was going to pick it up)
	if (playerMovementScript->itemClicked == this && App->input->GetMouseButtonDown(1) == KEY_DOWN &&
		!(App->scene->Intersects(closestPoint, myBboxName.c_str()) || itemName->Intersection(gameobject->UUID)))
	{
		playerMovementScript->stoppedGoingToItem = true;
		playerMovementScript->itemClicked = nullptr;
	}
	//if the player has gotten to the item by walking towards it, this variable will be true, so we pick up
	if (pickedUpViaPlayer)
	{
		PickupItem();
	}

	GameObject* myRenderGO = App->scene->FindGameObjectByName(myBboxName.c_str(), gameobject);
	if (myRenderGO != nullptr)
		myRender = (ComponentRenderer*)myRenderGO->GetComponent<ComponentRenderer>();

	if (myRender != nullptr)
	{
		myRender->highlighted = true;
		math::float4 color = itemName->GetColor(gameobject->UUID);
		myRender->highlightColor = math::float3(color.x, color.y, color.z);
	}

	
	if (App->scene->maincamera->frustum->Intersects(gameobject->GetBoundingBox()) && std::find(nameShowed.begin(), nameShowed.end(), gameobject->UUID) == nameShowed.end() && gameobject->isActive())
	{
		itemName->SetNameBar(gameobject->UUID, rarity);
		nameShowed.emplace_back(gameobject->UUID);
	}
	else if (!App->scene->maincamera->frustum->Intersects(gameobject->GetBoundingBox()) && std::find(nameShowed.begin(), nameShowed.end(), gameobject->UUID) != nameShowed.end())
	{
		nameShowed.erase(std::find(nameShowed.begin(), nameShowed.end(), gameobject->UUID));
		itemName->DisableName(gameobject->UUID);
	}

	if (App->scene->Intersects(closestPoint, myBboxName.c_str()) || itemName->Intersection(gameobject->UUID))
	{
		if (itemName != nullptr)
			itemName->Hovered(gameobject->UUID, true);

		if (myRender != nullptr)
			myRender->highlighted = true;

		if (changeItemCursorIcon && !App->ui->IsHover())
		{
			MouseController::ChangeCursorIcon(itemCursor);
			App->ui->SetIsItemHover(true);
			changeItemCursorIcon = false;
			changeStandarCursorIcon = true;
		}
	}
	
	else
	{
		if (myRender != nullptr)
		{
			//myRender->highlighted = false;
			itemName->Hovered(gameobject->UUID, false);

			if (changeStandarCursorIcon && !App->ui->IsHover())
			{
				MouseController::ChangeCursorIcon(gameStandarCursor);
				App->ui->SetIsItemHover(false);
				changeStandarCursorIcon = false;
				changeItemCursorIcon = true;
			}
		}
	}
}

void ItemPicker::Serialize(JSON_value* json) const
{
	assert(json != nullptr);
	json->AddString("name", name.c_str());
	json->AddString("sprite", sprite.c_str());
	json->AddInt("type", (int)type);
	json->AddInt("rarity", (int)rarity);
	json->AddInt("isEquipped", (int)item.isEquipped);
	json->AddFloat("health", item.stats.health);
	json->AddFloat("mana", item.stats.mana);
	json->AddInt("strength", item.stats.strength);
	json->AddInt("dexterity", item.stats.dexterity);
	json->AddFloat("hpRegen", item.stats.hpRegen);
	json->AddFloat("manaRegen", item.stats.manaRegen);
	json->AddString("itemCursor", itemCursor.c_str());
	json->AddUint("meshUID", item.meshUID);
	json->AddUint("materialUID", item.materialUID);
}

void ItemPicker::DeSerialize(JSON_value* json)
{
	assert(json != nullptr);
	name = json->GetString("name");
	sprite = json->GetString("sprite");
	type = (ItemType)json->GetInt("type");
	rarity = (ItemRarity)json->GetInt("rarity");
	item.isEquipped = json->GetInt("isEquipped");
	item.stats.health = json->GetFloat("health");
	item.stats.mana = json->GetFloat("mana");
	item.stats.strength = json->GetInt("strength");
	item.stats.dexterity = json->GetInt("dexterity");
	item.stats.hpRegen = json->GetFloat("hpRegen");
	item.stats.manaRegen = json->GetFloat("manaRegen");
	itemCursor = json->GetString("itemCursor", "Pick.cur");

	// Mesh
	unsigned resourceMeshUID = json->GetUint("meshUID");
	if (resourceMeshUID > 0u)
	{
		item.meshUID = resourceMeshUID;
		itemMesh =(ResourceMesh*)App->resManager->GetWithoutLoad(resourceMeshUID);
	}

	//Material
	unsigned resourceMaterialUID = json->GetUint("materialUID");
	if (resourceMaterialUID > 0u)
	{
		item.materialUID = resourceMaterialUID;
		itemMaterial = (ResourceMaterial*)App->resManager->GetWithoutLoad(resourceMaterialUID);
	}
}

void ItemPicker::SetItem(ItemType type, std::string name, std::string sprite)
{
	item.name = name;
	item.sprite = sprite;
	item.type = type;
}