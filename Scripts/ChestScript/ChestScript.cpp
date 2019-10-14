#include "ChestScript.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleTime.h"
#include "ModuleNavigation.h"
#include "MouseController.h"
#include "ModuleInput.h"

#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentAnimation.h"
#include "ComponentRenderer.h"
#include "ComponentAudioSource.h"

#include "LootDropScript.h"
#include "PlayerMovement.h"

#include "imgui.h"
#include "JSON.h"

ChestScript_API Script* CreateScript()
{
	ChestScript* instance = new ChestScript;
	return instance;
}

void ChestScript::Start()
{
	player = App->scene->FindGameObjectByTag(playerTag.c_str());
	if (player == nullptr)
	{
		LOG("The Player GO with tag %s couldn't be found \n", playerTag.c_str());
	}
	else
	{
		playerBbox = &App->scene->FindGameObjectByName("PlayerMesh", player)->bbox;
		if (playerBbox == nullptr)
		{
			LOG("The GameObject %s has no bbox attached \n", player->name.c_str());
		}

		playerMovementScript = player->GetComponent<PlayerMovement>();
	}

	anim = gameobject->GetComponent<ComponentAnimation>();
	if (anim == nullptr)
	{
		LOG("The GameObject %s has no Animation component attached \n", gameobject->name.c_str());
	}
	
	myRender = (ComponentRenderer*)gameobject->GetComponentInChildren(ComponentType::Renderer);

	if(myRender != nullptr)
		myBbox = &gameobject->bbox;
	
	// Look for LootDropScript
	lootDrop = gameobject->GetComponent<LootDropScript>();
	if (lootDrop != nullptr)
		lootDrop->positionOffset = lootPosition;

	GameObject* GO = nullptr;
	GO = App->scene->FindGameObjectByName("open_chest");
	if (GO != nullptr)
	{
		open_chest = GO->GetComponent<ComponentAudioSource>();
		assert(open_chest != nullptr);
	}
	else
	{
		LOG("Warning: open_chest game object not found");
	}
}

void ChestScript::Update()
{
	switch (state)
	{
	case chestState::CLOSED:
		// Check hover
		OnChestClosedHover();
		break;
	case chestState::OPENING:
		if (chestTimer > lootDelay)
		{
			// If chest has more than one item drop them in circle
			if (lootDrop->itemList.size() > 1)
				lootDrop->DropItemsInSemiCircle(lootRadius);
			else
				lootDrop->DropItems();

			state = chestState::OPENED;
		}
		else
		{
			chestTimer += App->time->gameDeltaTime;
		}
		break;
	default:
	case chestState::OPENED:
		if (myRender != nullptr)
			myRender->highlighted = false;
		break;
	}
}

void ChestScript::Expose(ImGuiContext* context)
{
	switch (state)
	{
	case chestState::CLOSED:	ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Closed");	break;
	case chestState::OPENING:	ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Opening");	break;
	case chestState::OPENED:	ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Opened");	break;
	default: break;
	}

	ImGui::Separator();
	ImGui::Text("Player:");
	char* goTag = new char[64];
	strcpy_s(goTag, strlen(playerTag.c_str()) + 1, playerTag.c_str());
	ImGui::InputText("Player Tag", goTag, 64);
	playerTag = goTag;
	delete[] goTag;

	ImGui::Text("Chest cursor:");
	char* pickCursorAux = new char[64];
	strcpy_s(pickCursorAux, strlen(pickCursor.c_str()) + 1, pickCursor.c_str());
	ImGui::InputText("pickCursor", pickCursorAux, 64);
	pickCursor = pickCursorAux;
	delete[] pickCursorAux;

	ImGui::Text("Loot Variables:");
	ImGui::DragFloat("Loot Delay", &lootDelay);
	ImGui::DragFloat3("Loot Position", (float*)&lootPosition);
	ImGui::DragFloat("Loot Radius", &lootRadius);
}

void ChestScript::Serialize(JSON_value* json) const
{
	assert(json != nullptr);
	json->AddString("playerTag", playerTag.c_str());
	json->AddString("pickCursor", pickCursor.c_str());
	json->AddUint("state", (unsigned)state);
	json->AddFloat3("lootPosition", lootPosition);
	json->AddFloat("lootDelay", lootDelay);
	json->AddFloat("lootRadius", lootRadius);
}

void ChestScript::DeSerialize(JSON_value* json)
{
	assert(json != nullptr);
	playerTag = json->GetString("playerTag", "Player");
	pickCursor = json->GetString("pickCursor", "Pick.cur");
	state = (chestState)json->GetUint("opened");
	lootPosition = json->GetFloat3("lootPosition");
	lootDelay = json->GetFloat("lootDelay", 2.5f);
	lootRadius = json->GetFloat("lootRadius", 100.0f);
}

void ChestScript::OnChestClosedHover()
{
	math::float3 closestPoint;
	fPoint mouse_point = App->input->GetMousePosition();
	math::float2 mouse = { mouse_point.x, mouse_point.y };
	std::list<GameObject*> intersects = App->scene->SceneRaycastHit(mouse);

	// First check if chest clicked (either the item mesh or its name)
	if ((App->scene->Intersects(closestPoint, myRender->gameobject->name.c_str()) &&
		App->input->GetMouseButtonDown(1) == KEY_DOWN))
	{
		// If player next to the item
		if (myRender->gameobject->bbox.Intersects(*playerBbox))
		{
			// Open chest:
			anim->SendTriggerToStateMachine("Open");
			
			open_chest->Play();

			if (lootDrop != nullptr)
				state = chestState::OPENING;
			else
				state = chestState::OPENED;
		}
		// If not, player goes towards it
		else
		{
			playerMovementScript->stoppedGoingToItem = false;
		}

		lastClickOnChest = true;
	}
	else if (App->input->GetMouseButtonDown(1) == KEY_DOWN)
	{
		lastClickOnChest = false;
	}

	// If player is next to the item and last click was done to the chest
	if (lastClickOnChest && myRender->gameobject->bbox.Intersects(*playerBbox))
	{
		// Open chest:
		anim->SendTriggerToStateMachine("Open");
		if (lootDrop != nullptr)
			state = chestState::OPENING;
		else
			state = chestState::OPENED;
	}

	// Highlight and cursor
	auto mesh = std::find(intersects.begin(), intersects.end(), myRender->gameobject);
	if (mesh != std::end(intersects) && *mesh == myRender->gameobject)
	{
		if (myRender != nullptr)
			myRender->highlighted = true;

		MouseController::ChangeCursorIcon(pickCursor);
	}
	else
	{
		if (myRender != nullptr)
			myRender->highlighted = false;

		MouseController::ChangeCursorIcon(gameStandarCursor);
	}
}
