#include "ExperienceController.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleTime.h"
#include "ModuleInput.h"

#include "GameObject.h"
#include "ComponentText.h"
#include "ComponentImage.h"

#include "SkillTreeController.h"

#include "JSON.h"
#include "imgui.h"

ExperienceController_API Script* CreateScript()
{
	ExperienceController* instance = new ExperienceController;
	return instance;
}

void ExperienceController::Start()
{
	GameObject* levelInventory = App->scene->FindGameObjectByName("LevelBackground");
	assert(levelInventory != nullptr);
	xpProgressInventory = App->scene->FindGameObjectByName("LevelProgressBar", levelInventory)->GetComponent<ComponentImage>();
	assert(xpProgressInventory != nullptr);
	xpProgressHUD = App->scene->FindGameObjectByName("XpProgress", gameobject)->GetComponent<ComponentImage>();
	assert(xpProgressHUD != nullptr);
	levelText = App->scene->FindGameObjectByName("LevelActual", levelInventory)->GetComponent<Text>();
	assert(levelText != nullptr);
	xpText = App->scene->FindGameObjectByName("LevelExperience", levelInventory)->GetComponent<Text>();
	assert(xpText != nullptr);
	levelUPGO = App->scene->FindGameObjectByName("LevelUP");
	assert(levelUPGO != nullptr);
	levelReached = App->scene->FindGameObjectByName("LevelReached", levelUPGO)->GetComponent<Text>();
	assert(levelReached != nullptr);
	

	skillTreeScript = App->scene->FindGameObjectByName("Skills")->GetComponent<SkillTreeController>();
}

void ExperienceController::Update()
{
	/*if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
	{
		AddXP(350);
	}*/

	if (levelUP)
	{
		currentTime += App->time->gameDeltaTime;
		if (currentTime >= timeShowing)
		{
			levelUPGO->SetActive(false);
			levelUP = false;
			currentTime = 0;
		}
	}
}

void ExperienceController::AddXP(int xp)
{
	totalXPAcumulated += xp;
	if (currentLevel < maxLevel)
	{
		updateXP = true;
		previousXP = currentXP;
		currentXP += xp;
		if (currentXP >= maxXPLevel)
		{
			while (currentXP >= maxXPLevel)
			{
				++currentLevel;
				levelUP = true;
				if (currentLevel == maxLevel)
				{
					currentXP = maxXPLevel;
					break;
				}
				currentXP -= maxXPLevel;
				maxXPLevel = levelsExp[currentLevel - 1];
				skillTreeScript->AddSkillPoint();
			}
			levelText->text = "LVL " + std::to_string(currentLevel);
			levelReached->text = "LEVEL " + std::to_string(currentLevel) + " REACHED";
			levelUPGO->SetActive(true);
		}
		int mask = (currentXP * 100) / maxXPLevel;
		xpText->text = std::to_string(currentXP) + "/" + std::to_string(maxXPLevel);
		xpProgressHUD->SetMaskAmount(mask);
		xpProgressInventory->SetMaskAmount(mask);
	}
}

void ExperienceController::Expose(ImGuiContext* context)
{
	ImGui::SetCurrentContext(context);

	ImGui::DragFloat("Time showing levelUp meassage", &timeShowing, 1.0f, 0.0f, 10.0f);
	int oldMaxLevel = maxLevel;
	if (ImGui::InputInt("Number of levels", &maxLevel, ImGuiInputTextFlags_EnterReturnsTrue))
	{
		if (maxLevel <= 0 || maxLevel > 23)
		{
			maxLevel = oldMaxLevel;
		}
	}

	for (int i = 0; i < maxLevel; ++i) {
		ImGui::PushID(i);
		ImGui::InputInt(("Level " + std::to_string(i + 1) + " XP: ").c_str(), &levelsExp[i]);
		ImGui::PopID();
	}
}

void ExperienceController::Serialize(JSON_value* json) const
{
	assert(json != nullptr);
	json->AddFloat("timeShowing", timeShowing);
	json->AddInt("numLevels", maxLevel);
	for (int i = 0; i < 23; ++i) {
		json->AddInt(std::to_string(i).c_str(), levelsExp[i]);
	}
}

void ExperienceController::DeSerialize(JSON_value* json)
{
	assert(json != nullptr);
	timeShowing = json->GetFloat("timeShowing", 3.0f);
	maxLevel = json->GetInt("numLevels", 23);
	for (int i = 0; i < 23; ++i) {
		levelsExp[i] = json->GetInt(std::to_string(i).c_str(), levelsExp[i]);
	}
}
