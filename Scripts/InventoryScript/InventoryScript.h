#ifndef  __InventoryScript_h__
#define  __InventoryScript_h__

#include "BaseScript.h"

#ifdef InventoryScript_EXPORTS
#define InventoryScript_API __declspec(dllexport)
#else
#define InventoryScript_API __declspec(dllimport)
#endif

#include <vector>
#include <list>
#include "Math/float2.h"
#include "Item.h"

class Component;
class GameObject;
class Transform2D;
class ComponentAudioSource;
class PlayerMovement;

#define TOTAL_SLOTS 24
#define INVENTARY_SLOTS 18
#define ASSIGNED_CONSUMABLES_SIZE 9

class InventoryScript_API InventoryScript : public Script
{
public:
	~InventoryScript();
	void Awake() override;
	void Start() override;
	void Update() override;

	inline virtual InventoryScript* Clone() const
	{
		return new InventoryScript(*this);
	}

	bool AddItem(Item item, unsigned amount = 1u);
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
	int ManageConsumableItemsQuantity(const Item& item, int value = 1);
	void ManageConsumableItemsQuantityText(const Item& item, int quantity);
	int GetItemIndexPosition(const Item& item);
	void HideConsumableItemText(int position);
	void UseItemConsumableOnPlayer(int itemPosition);

	std::vector<Component*> slotsTransform;
	std::vector<GameObject*> itemsSlots;
	std::vector<GameObject*> itemsSlotsNumbers;
	std::vector<std::pair<Item*, int>> items;
	std::vector<std::pair<std::string, int>> consumableItems; //name of the item, quantity

	std::string assignedConsumableItem[ASSIGNED_CONSUMABLES_SIZE] = { "", "", "", "", "", "", "", "", "" };

	GameObject* inventory = nullptr;
	GameObject* itemDesc = nullptr;
	Transform2D* menuPlayer = nullptr;
	ComponentImage* imageHover = nullptr;

	math::float2 initialitemPos = math::float2::zero;

	ComponentAudioSource* selectItemAudio;
	ComponentAudioSource* dropItemAudio;

	GameObject* player = nullptr;
	PlayerMovement* playerMovement = nullptr;

	bool skill = false;

	std::list<GameObject*> slotsToActivate;

public:
	bool itemGrabbed = false;
	std::list<int> equipedConsumablesToRemove;
};

extern "C" InventoryScript_API Script* CreateScript();

#endif __InventoryScript_h__
