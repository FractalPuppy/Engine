#include "ExperienceController.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleTime.h"
#include "ModuleInput.h"
#include "ModuleResourceManager.h"

#include "GameObject.h"
#include "ComponentText.h"
#include "ComponentImage.h"
#include "ComponentRenderer.h"

#include "SkillTreeController.h"
#include "PlayerMovement.h"

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

	GameObject* player = App->scene->FindGameObjectByTag("Player");
	if (player != nullptr)
	{
		playerScript = player->GetComponent<PlayerMovement>();
		if(playerScript == nullptr)
			LOG("PlayerMovement script couldn't be found \n");
	}
	else
	{
		LOG("Player couldn't be found \n");
	}

	GameObject* playerMesh = App->scene->FindGameObjectByName("PlayerMesh");
	if (playerMesh == nullptr)
	{
		LOG("PlayerMesh couldn't be found \n");
	}
	else
	{
		playerRender = playerMesh->GetComponent< ComponentRenderer>();

		if (playerRender == nullptr)
		{
			LOG("Player Render couldn't be found \n");
		}
		else
		{
			playerRender->highlightColor = expColor;
		}
	}
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

	// Dissolve effect
	if (useDissolveEffect && expDisolve)
	{
		if (dissolveTimer > (dissolveDuration/2.0f))	// Increment dissolve
		{
			dissolveTimer -= App->time->gameDeltaTime;
			playerRender->dissolveAmount = (dissolveTimer / dissolveDuration);
		}
		else if (dissolveTimer > 0.0f)					// Decrease dissolve
		{
			dissolveTimer -= App->time->gameDeltaTime;
			playerRender->dissolveAmount = 1.0f - (dissolveTimer / dissolveDuration);
		}
		else
		{
			// End Effect
			playerRender->dissolve = false;
			expDisolve = false;
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
				LevelUpStats(); // Upgrade stats
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

	if (useDissolveEffect && playerRender != nullptr)
	{
		// Play effect on player render
		expDisolve = true;
		dissolveTimer = dissolveDuration;
		playerRender->dissolve = true;
		playerRender->dissolveAmount = 0.0f;
		playerRender->borderAmount = borderAmount;
	}
}

void ExperienceController::LevelUpStats()
{
	// Upgrade stats
	PlayerStats* stats = &playerScript->stats;
	stats->health += healthIncrease;
	stats->mana += manaIncrease;
	stats->strength += strengthIncrease;
	stats->dexterity += dexterityIncrease;
}

void ExperienceController::Expose(ImGuiContext* context)
{
	ImGui::SetCurrentContext(context);

	ImGui::DragFloat("Time showing levelUp message", &timeShowing, 1.0f, 0.0f, 10.0f);
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

	ImGui::Separator();
	ImGui::Text("Dissolve Effect:");
	ImGui::Checkbox("Use Effect", &useDissolveEffect);
	if (useDissolveEffect)
	{
		ImGui::DragFloat("Duration", &dissolveDuration, 0.1f);
		ImGui::DragFloat("Border Amount", &borderAmount, 0.1f);
	}
	ImGui::Separator();
	ImGui::Text("Stat Increase on Lvl up:");
	ImGui::DragFloat("Health", &healthIncrease);
	ImGui::DragFloat("Mana", &manaIncrease);
	ImGui::DragInt("Strength", &strengthIncrease, 1.0f, 0);
	ImGui::DragInt("Dexterity", &dexterityIncrease, 1.0f, 0);
}

void ExperienceController::Serialize(JSON_value* json) const
{
	assert(json != nullptr);
	json->AddFloat("timeShowing", timeShowing);
	json->AddInt("numLevels", maxLevel);
	for (int i = 0; i < 23; ++i) {
		json->AddInt(std::to_string(i).c_str(), levelsExp[i]);
	}
	json->AddUint("useDissolveEffect", useDissolveEffect);
	if (useDissolveEffect)
	{
		json->AddFloat("dissolveDuration", dissolveDuration);
		json->AddFloat("borderAmount", borderAmount);
	}
	json->AddFloat("healthIncrease", healthIncrease);
	json->AddFloat("manaIncrease", manaIncrease);
	json->AddInt("strengthIncrease", strengthIncrease);
	json->AddInt("dexterityIncrease", dexterityIncrease);
}

void ExperienceController::DeSerialize(JSON_value* json)
{
	assert(json != nullptr);
	timeShowing = json->GetFloat("timeShowing", 3.0f);
	maxLevel = json->GetInt("numLevels", 23);
	for (int i = 0; i < 23; ++i) {
		levelsExp[i] = json->GetInt(std::to_string(i).c_str(), levelsExp[i]);
	}
	useDissolveEffect = json->GetUint("useDissolveEffect", 1.0f);
	dissolveDuration = json->GetFloat("dissolveDuration", 0.8f);
	borderAmount = json->GetFloat("borderAmount", 0.4f);
	healthIncrease = json->GetFloat("healthIncrease", 10.0f);
	manaIncrease = json->GetFloat("manaIncrease", 10.0f);
	strengthIncrease = json->GetInt("strengthIncrease", 5);
	dexterityIncrease = json->GetInt("dexterityIncrease", 1);
}

void ExperienceController::SaveExperience()
{
	PlayerPrefs::SetInt("currentLevel", currentLevel);
	PlayerPrefs::SetInt("currentXP", currentXP);
	PlayerPrefs::SetInt("totalXPAcumulated", totalXPAcumulated);
}
