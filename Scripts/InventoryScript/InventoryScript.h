#ifndef  __InventoryScript_h__
#define  __InventoryScript_h__

#include "BaseScript.h"

#ifdef InventoryScript_EXPORTS
#define InventoryScript_API __declspec(dllexport)
#else
#define InventoryScript_API __declspec(dllimport)
#endif

#include <vector>
#include "Math/float2.h"
#include "Item.h"

class Component;
class GameObject;
class Transform2D;
class ComponentAudioSource;
class PlayerMovement;

#define TOTAL_SLOTS 24
#define INVENTARY_SLOTS 18

class InventoryScript_API InventoryScript : public Script
{
public:
	void Awake() override;
	void Start() override;
	void Update() override;

	inline virtual InventoryScript* Clone() const
	{
		return new InventoryScript(*this);
	}

	bool AddItem(Item* item);
	std::vector<Item> GetQuickItems();
	int GetCurrentQuantity(const Item& item);
	int GetCurrentQuantity(std::string itemName);
	void AssignConsumableItem(const Item& item, int position);
	void UnAssignConsumableItem(int position);
	int ConsumeItemsController();
	void SaveInventory();
	void LoadInventory();

private:
	void showDescription(int i);
	int ManageConsumableItemsQuantity(const Item& item);
	void ManageConsumableItemsQuantityText(const Item& item, int quantity);
	int GetItemIndexPosition(const Item& item);
	void HideConsumableItemText(int position);
	void UseItemConsumableOnPlayer(int itemPosition);

	std::vector<Component*> slotsTransform;
	std::vector<GameObject*> itemsSlots;
	std::vector<GameObject*> itemsSlotsNumbers;
	std::vector<std::pair<Item*, int>> items;
	std::vector<std::pair<std::string, int>> consumableItems; //name of the item, quantity

	std::string assignedConsumableItem[9] = { "", "", "", "", "", "", "", "", "" };

	GameObject* inventory = nullptr;
	GameObject* itemDesc = nullptr;
	Transform2D* menuPlayer = nullptr;
	ComponentImage* imageHover = nullptr;

	math::float2 initialitemPos = math::float2::zero;

	ComponentAudioSource* selectItemAudio;
	ComponentAudioSource* dropItemAudio;

	PlayerMovement* playerMovement = nullptr;

	bool itemGrabbed = false;
	bool skill = false;

};

extern "C" InventoryScript_API Script* CreateScript();

#endif __InventoryScript_h__
