#include "ExperienceController.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleTime.h"
#include "ModuleInput.h"

#include "GameObject.h"
#include "ComponentText.h"
#include "ComponentImage.h"
#include "ComponentRenderer.h"

#include "SkillTreeController.h"

#include "JSON.h"
#include "imgui.h"

#include "PlayerPrefs.h"

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
	xpTextMax = App->scene->FindGameObjectByName("LevelExperienceMax", levelInventory)->GetComponent<Text>();
	assert(xpTextMax != nullptr);
	levelUPGO = App->scene->FindGameObjectByName("LevelUP");
	assert(levelUPGO != nullptr);
	levelReached = App->scene->FindGameObjectByName("LevelReached", levelUPGO)->GetComponent<Text>();
	assert(levelReached != nullptr);
	
	GameObject* levelUpFloorGO = App->scene->FindGameObjectByName("LevelUpFloorMesh");
	if (levelUpFloorGO != nullptr)
	{
		levelUpFloorMesh = levelUpFloorGO->GetComponent<ComponentRenderer>();
		levelUpParticles = App->scene->FindGameObjectByName("LevelUpParticles");
	}


	currentXP = PlayerPrefs::GetInt("currentXP", 0);
	totalXPAcumulated = PlayerPrefs::GetInt("totalXPAcumulated", 0);
	currentLevel = PlayerPrefs::GetInt("currentLevel", 1);
	maxXPLevel = levelsExp[currentLevel - 1];

	int mask = (currentXP * 100) / maxXPLevel;
	xpText->text = std::to_string(currentXP);
	xpTextMax->text = std::to_string(maxXPLevel);
	xpProgressHUD->SetMaskAmount(mask);
	xpProgressInventory->SetMaskAmount(mask);
	levelText->text = std::to_string(currentLevel);
	
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
		if (levelUpFloorMesh->animationEnded)
		{
			levelUpFloorMesh->gameobject->SetActive(false);
			levelUpParticles->SetActive(false);
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
				App->scene->FindGameObjectByName("NewSkillPoint")->SetActive(true);
				//playermovement->addStats (subir de lvl)
			}
			levelText->text = std::to_string(currentLevel);
			levelReached->text = "LEVEL " + std::to_string(currentLevel) + " REACHED";
			levelUPGO->SetActive(true);
			levelUpFloorMesh->gameobject->SetActive(true);
			levelUpFloorMesh->ResetAnimation();
			levelUpParticles->SetActive(true);
		}
		int mask = (currentXP * 100) / maxXPLevel;
		xpText->text = std::to_string(currentXP);
		xpTextMax->text = std::to_string(maxXPLevel);
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

void ExperienceController::SaveExperience()
{
	PlayerPrefs::SetInt("currentLevel", currentLevel);
	PlayerPrefs::SetInt("currentXP", currentXP);
	PlayerPrefs::SetInt("totalXPAcumulated", totalXPAcumulated);
}
