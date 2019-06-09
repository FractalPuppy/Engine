#include "SkillTreeController.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleInput.h"

#include "GameObject.h"
#include "ComponentText.h"
#include "ComponentImage.h"


SkillTreeController_API Script* CreateScript()
{
	SkillTreeController* instance = new SkillTreeController;
	return instance;
}

void SkillTreeController::Start()
{
	skillPointsLabel = App->scene->FindGameObjectByName(gameobject, "SkillPoints")->GetComponent<Text>();
	tree = App->scene->FindGameObjectByName(gameobject, "SkillsTree");
	skillPoints = 3;
	skillPointsLabel->text = std::to_string(skillPoints);
}

void SkillTreeController::Update()
{
	ComponentImage* image = tree->GetComponent<ComponentImage>();

	if (image->isHovered && App->input->GetMouseButtonDown(1) == KEY_DOWN && skillPoints > 0)
	{
		--skillPoints;
		skillPointsLabel->text = std::to_string(skillPoints);
	}
}

void SkillTreeController::AddSkillPoint()
{
	++skillPoints;
	skillPointsLabel->text = std::to_string(skillPoints);
}
