#include "EnemyControllerScript.h"

#include "Application.h"
#include "ModuleScene.h"

#include "GameObject.h"
#include "ComponentRenderer.h"

#include "imgui.h"
#include "JSON.h"

EnemyControllerScript_API Script* CreateScript()
{
	EnemyControllerScript* instance = new EnemyControllerScript;
	return instance;
}

void EnemyControllerScript::Start()
{
	player = App->scene->FindGameObjectByName(App->scene->root, playerName.c_str());
	myRender = (ComponentRenderer*)App->scene->FindGameObjectByName(gameobject, myBboxName.c_str())->GetComponentOld(ComponentType::Renderer);
	myBbox = &App->scene->FindGameObjectByName(gameobject, myBboxName.c_str())->bbox;
	playerBbox = &App->scene->FindGameObjectByName(player, playerBboxName.c_str())->bbox;
}

void EnemyControllerScript::Expose(ImGuiContext * context)
{
	char* bboxName = new char[64];
	strcpy_s(bboxName, strlen(myBboxName.c_str()) + 1, myBboxName.c_str());
	ImGui::InputText("My BBox Name", bboxName, 64);
	myBboxName = bboxName;
	delete[] bboxName;

	ImGui::InputInt("Health", &health);

	switch (enemyState)
	{
	case EnemyState::WAIT:		ImGui::TextColored(ImVec4(1, 1, 0, 1), "State: Wait");		break;
	case EnemyState::STANDUP:	ImGui::TextColored(ImVec4(1, 1, 0, 1), "State: Stand-Up");	break;
	case EnemyState::CHASE:		ImGui::TextColored(ImVec4(1, 1, 0, 1), "State: Chase");		break;
	case EnemyState::RETURN:	ImGui::TextColored(ImVec4(1, 1, 0, 1), "State: Return");	break;
	case EnemyState::LAYDOWN:	ImGui::TextColored(ImVec4(1, 1, 0, 1), "State: Laydown");	break;
	case EnemyState::ATTACK:	ImGui::TextColored(ImVec4(1, 1, 0, 1), "State: Attack");	break;
	case EnemyState::COOLDOWN:	ImGui::TextColored(ImVec4(1, 1, 0, 1), "State: Cooldown");	break;
	case EnemyState::DEAD:		ImGui::TextColored(ImVec4(1, 1, 0, 1), "State: Dead");		break;
	default:
		break;
	}

	ImGui::Separator();
	ImGui::Text("Player:");
	char* goName = new char[64];
	strcpy_s(goName, strlen(playerName.c_str()) + 1, playerName.c_str());
	ImGui::InputText("playerName", goName, 64);
	playerName = goName;
	delete[] goName;

	char* targetBboxName = new char[64];
	strcpy_s(targetBboxName, strlen(playerBboxName.c_str()) + 1, playerBboxName.c_str());
	ImGui::InputText("Player BBox Name", targetBboxName, 64);
	playerBboxName = targetBboxName;
	delete[] targetBboxName;
}

void EnemyControllerScript::Serialize(JSON_value* json) const
{
	assert(json != nullptr);
	json->AddString("playerName", playerName.c_str());
	json->AddString("playerBboxName", playerBboxName.c_str());
	json->AddString("myBboxName", myBboxName.c_str());
}

void EnemyControllerScript::DeSerialize(JSON_value* json)
{
	assert(json != nullptr);
	playerName = json->GetString("playerName");
	playerBboxName = json->GetString("playerBboxName");
	myBboxName = json->GetString("myBboxName");
}

void EnemyControllerScript::TakeDamage(unsigned damage)
{
	if (health - damage < 0)
	{
		health = 0;
		enemyState = EnemyState::DEAD;
	}
	else
	{
		health -= damage;
	}
}
