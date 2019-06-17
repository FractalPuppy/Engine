#include "EquipPopupController.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleInput.h"

#include "GameObject.h"
#include "ComponentButton.h"
#include "ComponentText.h"
#include "ComponentImage.h"

EquipPopupController_API Script* CreateScript()
{
	EquipPopupController* instance = new EquipPopupController;
	return instance;
}

void EquipPopupController::Start()
{
	popupGO = App->scene->FindGameObjectByName("PopUpBackground");
	assert(popupGO != nullptr);

	background = popupGO->GetComponent<ComponentImage>();
	assert(background != nullptr);

	items = App->scene->FindGameObjectByName("PopUpItems",popupGO)->GetComponent<Button>();
	assert(items != nullptr);

	skills = App->scene->FindGameObjectByName("PopUpSkills", popupGO)->GetComponent<Button>();
	assert(skills != nullptr);

	unequip = App->scene->FindGameObjectByName("PopUpUnequip", popupGO)->GetComponent<Button>();
	assert(unequip != nullptr);

	title = App->scene->FindGameObjectByName("PopUpEquipTitle", popupGO)->GetComponent<Text>();
	assert(title != nullptr);

	GameObject* HUD = App->scene->FindGameObjectByName("GameHUB");
	assert(HUD != nullptr);
	hudButtons.emplace_back(App->scene->FindGameObjectByName("One", HUD)->GetComponent<Button>());
	hudButtons.emplace_back(App->scene->FindGameObjectByName("Two", HUD)->GetComponent<Button>());
	hudButtons.emplace_back(App->scene->FindGameObjectByName("Three", HUD)->GetComponent<Button>());
	hudButtons.emplace_back(App->scene->FindGameObjectByName("For", HUD)->GetComponent<Button>());
	hudButtons.emplace_back(App->scene->FindGameObjectByName("Q", HUD)->GetComponent<Button>());
	hudButtons.emplace_back(App->scene->FindGameObjectByName("W", HUD)->GetComponent<Button>());
	hudButtons.emplace_back(App->scene->FindGameObjectByName("E", HUD)->GetComponent<Button>());
	hudButtons.emplace_back(App->scene->FindGameObjectByName("R", HUD)->GetComponent<Button>());
	//One,Two,Three,For,Q,W,E,R --> GameHUB
}

void EquipPopupController::Update()
{
	if (popupGO->isActive() && !background->isHovered && !items->IsHovered() && !skills->IsHovered() && (App->input->GetMouseButtonDown(1) == KEY_DOWN || App->input->GetMouseButtonDown(3) == KEY_DOWN))
	{
		popupGO->SetActive(false);
		return;
	}

	for (int i = 0; i < 8; ++i)
	{
		if (hudButtons[i]->IsHovered() && App->input->GetMouseButtonDown(3) == KEY_DOWN) {
			activeButton = hudButtons[i];
			if (!popupGO->isActive())
			{
				FillSlots();
				popupGO->SetActive(true);
			}
			break;
		}
	}

	if (!popupGO->isActive()) { return; }

	if(items->IsPressed())
	{
		title->text = "Items";
	}

	if (skills->IsPressed())
	{
		title->text = "Skills";
	}
}

void EquipPopupController::FillSlots()
{

}
