#ifndef  __EquipPopupController_h__
#define  __EquipPopupController_h__

#include "BaseScript.h"
#include <vector>

#ifdef EquipPopupController_EXPORTS
#define EquipPopupController_API __declspec(dllexport)
#else
#define EquipPopupController_API __declspec(dllimport)
#endif

#define POPUP_SKILLS_SLOTS 11
#define POPUP_ITEMS_SLOTS 18
#define HUD_BUTTONS 9

class GameObject;
class Text;
class ComponentImage;
class Button;
class Item;
class Skill;
class SkillTreeController;
class InventoryScript;
class PlayerMovement;
class Transform2D;

class EquipPopupController_API EquipPopupController : public Script
{

	void Start() override;
	void Update() override;

	inline virtual EquipPopupController* Clone() const
	{
		return new EquipPopupController(*this);
	}
public:
	void SavePopUp();


private:

	void Assign(int i);
	void MoveNumber(int pos);
	void RemoveEquiped();
	void ChangePopUpSlots();
	void FillLists();
	void FillSkillSlots();
	void FillItemSlots();
	void CleanButton();
	void LoadPopUp();


	InventoryScript* inventory = nullptr;
	SkillTreeController* skillTree = nullptr;
	PlayerMovement* player = nullptr;

	GameObject* popupGOSkills = nullptr;
	GameObject* popupGOItems = nullptr;
	ComponentImage* backgroundSkills = nullptr;
	ComponentImage* backgroundItems = nullptr;
	Button* items = nullptr;
	Button* skills = nullptr;
	Button* unequip = nullptr;
	std::vector<GameObject*> slotsSkills;
	std::vector<GameObject*> slotsItems;
	Transform2D* hoverTransform = nullptr;

	std::vector<ComponentImage*> hudImageSlots;
	std::vector<Button*> hudButtons;
	std::vector<std::pair<Transform2D*, bool>> hudButtonsText;
	std::vector<Text*> hudConsumibleItemsQuantity;
	int activeButton = -1;

	std::vector<Item> itemsList;
	std::vector<Skill> skillsList;

	std::vector<std::pair<int, Skill>> skillsEquiped;
	std::vector<std::pair<int, Item>> itemsEquiped;
};

extern "C" EquipPopupController_API Script* CreateScript();

#endif __EquipPopupController_h__
