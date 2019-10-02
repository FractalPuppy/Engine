#include "ChestScript.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleTime.h"

#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentAnimation.h"
#include "ComponentRenderer.h"

#include "LootDropScript.h"

#include "imgui.h"
#include "JSON.h"

ChestScript_API Script* CreateScript()
{
	ChestScript* instance = new ChestScript;
	return instance;
}

void ChestScript::Start()
{
	player = App->scene->FindGameObjectByName(playerName.c_str());
	playerBbox = &App->scene->FindGameObjectByName(playerBboxName.c_str(), player)->bbox;

	anim = gameobject->GetComponent<ComponentAnimation>();
	if (anim == nullptr)
	{
		LOG("The GameObject %s has no Animation component attached \n", gameobject->name);
	}
	
	myRender = (App->scene->FindGameObjectByName(myBboxName.c_str(), gameobject))->GetComponent<ComponentRenderer>();

	if(myRender != nullptr)
		myBbox = &App->scene->FindGameObjectByName(myBboxName.c_str(), gameobject)->bbox;
	
	// Look for LootDropScript
	lootDrop = gameobject->GetComponent<LootDropScript>();
	if (lootDrop != nullptr)
		lootDrop->positionOffset = lootPosition;
}

void ChestScript::Update()
{
	switch (state)
	{
	case chestState::CLOSED:
		// Check collision with player
		if (myBbox != nullptr && myBbox->Intersects(*playerBbox))
		{
			// Open chest:
			anim->SendTriggerToStateMachine("Open");
			if (lootDrop != nullptr)
				state = chestState::OPENING;
			else
				state = chestState::OPENED;
			
		}
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
		break;
	}
}

void ChestScript::Expose(ImGuiContext* context)
{
	char* bboxName = new char[64];
	strcpy_s(bboxName, strlen(myBboxName.c_str()) + 1, myBboxName.c_str());
	ImGui::InputText("My BBox Name", bboxName, 64);
	myBboxName = bboxName;
	delete[] bboxName;

	switch (state)
	{
	case chestState::CLOSED:	ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Closed");	break;
	case chestState::OPENING:	ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Opening");	break;
	case chestState::OPENED:	ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Opened");	break;
	default: break;
	}

	ImGui::Separator();
	ImGui::Text("Player:");
	char* goName = new char[64];
	strcpy_s(goName, strlen(playerName.c_str()) + 1, playerName.c_str());
	ImGui::InputText("Player Name", goName, 64);
	playerName = goName;
	delete[] goName;

	char* targetBboxName = new char[64];
	strcpy_s(targetBboxName, strlen(playerBboxName.c_str()) + 1, playerBboxName.c_str());
	ImGui::InputText("Player BBox Name", targetBboxName, 64);
	playerBboxName = targetBboxName;
	delete[] targetBboxName;

	char* spawnName = new char[64];
	strcpy_s(spawnName, strlen(spawnGOName.c_str()) + 1, spawnGOName.c_str());
	ImGui::InputText("GO to spawn Name", spawnName, 64);
	spawnGOName = spawnName;
	delete[] spawnName;

	ImGui::Text("Loot Variables:");
	ImGui::DragFloat("Loot Delay", &lootDelay);
	ImGui::DragFloat3("Loot Position", (float*)&lootPosition);
	ImGui::DragFloat("Loot Radius", &lootRadius);
}

void ChestScript::Serialize(JSON_value* json) const
{
	assert(json != nullptr);
	json->AddString("playerName", playerName.c_str());
	json->AddString("playerBboxName", playerBboxName.c_str());
	json->AddString("myBboxName", myBboxName.c_str());
	json->AddString("spawnGOName", spawnGOName.c_str());
	json->AddUint("state", (unsigned)state);
	json->AddFloat3("lootPosition", lootPosition);
	json->AddFloat("lootDelay", lootDelay);
	json->AddFloat("lootRadius", lootRadius);
}

void ChestScript::DeSerialize(JSON_value* json)
{
	assert(json != nullptr);
	playerName = json->GetString("playerName");
	playerBboxName = json->GetString("playerBboxName");
	myBboxName = json->GetString("myBboxName");
	spawnGOName = json->GetString("spawnGOName");
	state = (chestState)json->GetUint("opened");
	lootPosition = json->GetFloat3("lootPosition");
	lootDelay = json->GetFloat("lootDelay", 2.5f);
	lootRadius = json->GetFloat("lootRadius", 100.0f);
}