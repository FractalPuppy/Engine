#include "SkillTreeController.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleInput.h"
#include "ModuleResourceManager.h"

#include "GameObject.h"
#include "Component.h"
#include "ComponentText.h"
#include "ComponentImage.h"
#include "ComponentTransform2D.h"

#include "imgui.h"
#include "JSON.h"
#include "Resource.h"
#include "Math/float4.h"
#include "Math/float2.h"
#include "ResourceTexture.h"

#include "PlayerPrefs.h"

#define None "None Selected"

#pragma warning(disable : 4996)

SkillTreeController_API Script* CreateScript()
{
	SkillTreeController* instance = new SkillTreeController;
	return instance;
}

void SkillTreeController::Start()
{
	std::list<GameObject*> list = App->scene->FindGameObjectByName("SkillSlots", gameobject)->children;
	skillUI = { std::begin(list), std::end(list) };

	connUI = App->scene->FindGameObjectByName("SkillConnections", gameobject)->GetComponentsInChildren(ComponentType::Image);

	skillPointsLabel = App->scene->FindGameObjectByName("SkillPoints", gameobject)->GetComponent<Text>();
	assert(skillPointsLabel != nullptr);
	skillPoints = PlayerPrefs::GetInt("SkillPoints", initialSkillPoints);
	skillPointsLabel->text = std::to_string(skillPoints);

	hoverTransform = App->scene->FindGameObjectByName("SkillHover", gameobject)->GetComponent<Transform2D>();
	assert(hoverTransform != nullptr);

	skillInfo = App->scene->FindGameObjectByName("SkillInfo", gameobject);
	assert(skillInfo != nullptr);
	skillInfoName = App->scene->FindGameObjectByName("SkillInfoName", skillInfo)->GetComponent<Text>();
	skillInfoDescription = App->scene->FindGameObjectByName("SkillInfoDescription", skillInfo)->GetComponent<Text>();
	skillInfoManaCostText = App->scene->FindGameObjectByName("SkillInfoManaCostText", skillInfo)->GetComponent<Text>();
	skillInfoIcon = App->scene->FindGameObjectByName("SkillInfoIcon", skillInfo)->GetComponent<ComponentImage>();


	for (int i = 0; i < NUM_SKILLS; ++i)
	{
		skillList[i].currentLevel = PlayerPrefs::GetInt(("Skill" + std::to_string(i)).c_str(), 0);
		((Text*)skillUI[i]->GetComponentInChildren(ComponentType::Text))->text = std::to_string(skillList[i].currentLevel) + "/" + std::to_string(skillList[i].maxLevels);

		if (skillList[i].currentLevel > 0)
		{
			skillList[i].available = true;
		}

		UnlockNextLevel(i);

		if (!skillList[i].locked)
		{
			skillUI[i]->GetComponent<ComponentImage>()->texture = (ResourceTexture*)App->resManager->GetByName("Activated_Hability_container", TYPE::TEXTURE);

			if (skillUI[i]->children.size() > 1)
			{
				if (skillList[i].available) {
					((ComponentImage*)(skillUI[i]->children.front())->GetComponentInChildren(ComponentType::Image))->UpdateTexture(skillList[i].spriteActive->GetName());
				}
				else {
					((ComponentImage*)(skillUI[i]->children.front())->GetComponentInChildren(ComponentType::Image))->texture = skillList[i].spriteInactive;
				}
			}
			((Text*)skillUI[i]->GetComponentInChildren(ComponentType::Text))->color = math::float4(0, 0, 0, 0);
		}
	}
}



void SkillTreeController::Update()
{
	if (!gameobject->isActive())
	{
		if (item) App->scene->FindGameObjectByName("NewItem")->SetActive(true);
		item = false;
		return;
	}

	hoverTransform->gameobject->SetActive(false);
	App->scene->FindGameObjectByName("NewSkillPoint")->SetActive(false);
	if (App->scene->FindGameObjectByName("NewItem")->isActive())
	{
		item = true;
		App->scene->FindGameObjectByName("NewItem")->SetActive(false);
	}
	skillInfo->SetActive(false);

	for (int i = 0; i < NUM_SKILLS; ++i)
	{
		if (!skillList[i].locked && skillUI[i]->GetComponent<ComponentImage>()->isHovered)
		{
			math::float2 pos = skillUI[i]->children.front()->GetComponent<Transform2D>()->getPosition();
			math::float2 newPos = math::float2(pos.x, pos.y);
			hoverTransform->SetPositionUsingAligment(newPos);
			hoverTransform->gameobject->SetActive(true);
			skillInfoName->text = skillList[i].name;
			skillInfoDescription->text = skillList[i].description;
			skillInfoManaCostText->text = std::to_string(skillList[i].mana);
			skillInfoIcon->UpdateTexture(skillList[i].spriteActive->GetName());
			skillInfo->SetActive(true);
			if (skillPoints > 0 && skillList[i].currentLevel < skillList[i].maxLevels && App->input->GetMouseButtonDown(1) == KEY_DOWN)
			{
				++skillList[i].currentLevel;
				skillList[i].available = true;
				((ComponentImage*)(skillUI[i]->children.front())->GetComponentInChildren(ComponentType::Image))->UpdateTexture(skillList[i].spriteActive->GetName());
				((Text*)skillUI[i]->GetComponentInChildren(ComponentType::Text))->text = std::to_string(skillList[i].currentLevel) + "/" + std::to_string(skillList[i].maxLevels);
				--skillPoints;
				skillPointsLabel->text = std::to_string(skillPoints);
				UnlockNextLevel(i);
			}
		}
	}
}

void SkillTreeController::UnlockNextLevel(int i)
{
	if (skillList[i].currentLevel == skillList[i].maxLevels && skillList[i].nextSkill >= 0)
	{
		if (skillList[i].connection == 0 || skillList[i].connection == 3 || skillList[i].connection == 4)
		{
			((ComponentImage*)connUI[skillList[i].connection])->texture = (ResourceTexture*)App->resManager->GetByName("Activated_Bar_Short", TYPE::TEXTURE);
		}
		else
		{
			((ComponentImage*)connUI[skillList[i].connection])->texture = (ResourceTexture*)App->resManager->GetByName("Activated_Bar", TYPE::TEXTURE);
		}
		skillList[skillList[i].nextSkill].locked = false;
		skillUI[skillList[i].nextSkill]->GetComponent<ComponentImage>()->texture = (ResourceTexture*)App->resManager->GetByName("Activated_Hability_container", TYPE::TEXTURE);
		((Text*)skillUI[skillList[i].nextSkill]->GetComponentInChildren(ComponentType::Text))->color = math::float4(0, 0, 0, 0);
		if (skillUI[skillList[i].nextSkill]->children.size() > 1)
		{
			((ComponentImage*)skillUI[skillList[i].nextSkill]->children.front()->GetComponentInChildren(ComponentType::Image))->texture = skillList[skillList[i].nextSkill].spriteInactive;
		}
	}
}

void SkillTreeController::AddSkillPoint()
{
	++skillPoints;
	skillPointsLabel->text = std::to_string(skillPoints);
}


std::vector<Skill> SkillTreeController::GetActiveSkills()
{
	std::vector<Skill> skillsToReturn;
	for (int i = 0; i < NUM_SKILLS; ++i)
	{
		if (skillList[i].available)
		{
			skillsToReturn.emplace_back(skillList[i]);
		}
	}

	return skillsToReturn;
}


void SkillTreeController::Expose(ImGuiContext* context)
{

	ImGui::SetCurrentContext(context);

	if (textureFiles.empty())
	{
		if(App != nullptr)
			textureFiles = App->resManager->GetResourceNamesList(TYPE::TEXTURE, true);
	}

	if (ImGui::Button("Refresh textures List"))
	{
		textureFiles.clear();
		textureFiles = App->resManager->GetResourceNamesList(TYPE::TEXTURE, true);
	}

	ImGui::InputInt("Initial Skill Points", &initialSkillPoints);

	for (int i = 0; i != NUM_SKILLS; ++i) {

		skillList[i].id = i;

		ImGui::PushID(i);
		if (ImGui::CollapsingHeader(skillList[i].name.c_str()))
		{
			ImGui::Text("Skill id: %i", skillList[i].id);

			if (ImGui::BeginCombo("Texture Active", skillList[i].spriteActive != nullptr ? skillList[i].spriteActive->GetName() : None))
			{
				bool none_selected = (skillList[i].spriteActive == nullptr);
				if (ImGui::Selectable(None, none_selected))
				{
					if (skillList[i].spriteActive != nullptr)
					{
						App->resManager->DeleteResource(skillList[i].spriteActive->GetUID());
						skillList[i].spriteActive = nullptr;
					}
				}
				if (none_selected)
					ImGui::SetItemDefaultFocus();
				for (int n = 0; n < textureFiles.size(); n++)
				{
					bool is_selected = (skillList[i].spriteActive != nullptr && (std::string(skillList[i].spriteActive->GetName()) == textureFiles[n]));
					if (ImGui::Selectable(textureFiles[n].c_str(), is_selected) && !is_selected)
					{
						// Delete previous texture
						if (skillList[i].spriteActive != nullptr)
							App->resManager->DeleteResource(skillList[i].spriteActive->GetUID());

						skillList[i].spriteActive = (ResourceTexture*)App->resManager->GetByName(textureFiles[n].c_str(), TYPE::TEXTURE);
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			if (ImGui::BeginCombo("Texture inactive", skillList[i].spriteInactive != nullptr ? skillList[i].spriteInactive->GetName() : None))
			{
				bool none_selected = (skillList[i].spriteInactive == nullptr);
				if (ImGui::Selectable(None, none_selected))
				{
					if (skillList[i].spriteInactive != nullptr)
					{
						App->resManager->DeleteResource(skillList[i].spriteInactive->GetUID());
						skillList[i].spriteInactive = nullptr;
					}
				}
				if (none_selected)
					ImGui::SetItemDefaultFocus();
				for (int n = 0; n < textureFiles.size(); n++)
				{
					bool is_selected = (skillList[i].spriteInactive != nullptr && (std::string(skillList[i].spriteInactive->GetName()) == textureFiles[n]));
					if (ImGui::Selectable(textureFiles[n].c_str(), is_selected) && !is_selected)
					{
						// Delete previous texture
						if (skillList[i].spriteInactive != nullptr)
							App->resManager->DeleteResource(skillList[i].spriteInactive->GetUID());

						skillList[i].spriteInactive = (ResourceTexture*)App->resManager->GetByName(textureFiles[n].c_str(), TYPE::TEXTURE);
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			char* imguiText = new char[64];
			strcpy(imguiText, skillList[i].name.c_str());
			if (ImGui::InputText("##", imguiText, 64, ImGuiInputTextFlags_EnterReturnsTrue))
				skillList[i].name = imguiText;
			delete[] imguiText;

			char* imguiTextD = new char[300];
			strcpy(imguiTextD, skillList[i].description.c_str());
			if (ImGui::InputText("Description", imguiTextD, 300))
				skillList[i].description = imguiTextD;
			delete[] imguiTextD;

			ImGui::InputInt("Num. Levels", &skillList[i].maxLevels);
			ImGui::InputInt("Next skill", &skillList[i].nextSkill);
			ImGui::InputInt("Connection", &skillList[i].connection);
			ImGui::InputInt("Mana Cost", &skillList[i].mana);

			ImGui::Checkbox("Locked", &skillList[i].locked);
		}
		ImGui::PopID();
	}

}


void SkillTreeController::Serialize(JSON_value* json) const
{
	assert(json != nullptr);

	JSON_value *skillsJson = json->CreateValue(rapidjson::kArrayType);
	for (auto &skill : skillList)
	{
		JSON_value *skillJSON = skillsJson->CreateValue();
		skillJSON->AddInt("id", skill.id);
		skillJSON->AddString("name", skill.name.c_str());
		skillJSON->AddString("description", skill.description.c_str());
		skillJSON->AddInt("locked", skill.locked);
		skillJSON->AddInt("maxLevel", skill.maxLevels);
		skillJSON->AddInt("nextSkill", skill.nextSkill);
		skillJSON->AddInt("connection", skill.connection);
		skillJSON->AddInt("mana", skill.mana);
		skillJSON->AddUint("activeTextureUID", (skill.spriteActive != nullptr) ? skill.spriteActive->GetUID() : 0u);
		skillJSON->AddUint("inactiveTextureUID", (skill.spriteInactive != nullptr) ? skill.spriteInactive->GetUID() : 0u);
		skillsJson->AddValue("", *skillJSON);
	}

	json->AddValue("skills", *skillsJson);
}

void SkillTreeController::DeSerialize(JSON_value* json)
{
	assert(json != nullptr);
	JSON_value* skillsJSON = json->GetValue("skills");
	for (unsigned i = 0; i < skillsJSON->Size(); i++)
	{
		JSON_value* skillJSON = skillsJSON->GetValue(i);
		skillList[i].id = skillJSON->GetInt("id");
		skillList[i].name = skillJSON->GetString("name");
		skillList[i].description = skillJSON->GetString("description");
		skillList[i].locked = skillJSON->GetInt("locked");
		skillList[i].maxLevels = skillJSON->GetInt("maxLevel");
		skillList[i].nextSkill = skillJSON->GetInt("nextSkill");
		skillList[i].connection = skillJSON->GetInt("connection", -1);
		skillList[i].mana = skillJSON->GetInt("mana", 0);
		unsigned uid = skillJSON->GetUint("activeTextureUID");
		skillList[i].spriteActive = (ResourceTexture*)App->resManager->Get(uid);
		unsigned uidIn = skillJSON->GetUint("inactiveTextureUID");
		skillList[i].spriteInactive = (ResourceTexture*)App->resManager->Get(uidIn);
	}
}

void SkillTreeController::SaveSkillTree()
{
	PlayerPrefs::SetInt("SkillPoints", skillPoints);
	for (int i = 0; i < NUM_SKILLS; ++i)
	{
		PlayerPrefs::SetInt(("Skill" + std::to_string(skillList[i].id)).c_str(), skillList[i].currentLevel);
	}
}