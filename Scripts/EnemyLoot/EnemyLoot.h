#ifndef  __EnemyLoot_h__
#define  __EnemyLoot_h__

#include "BaseScript.h"

#ifdef EnemyLoot_EXPORTS
#define EnemyLoot_API __declspec(dllexport)
#else
#define EnemyLoot_API __declspec(dllimport)
#endif

#include <vector>
#include "Item.h"
#include "imgui.h"


#include "Math\float3.h"

class GameObject;
class Script;
class ItemPicker;

class EnemyLoot_API EnemyLoot : public Script
{
public:
	void Start() override;
	void Update() override;
	void GenerateLoot();

private:

	std::vector<std::pair<GameObject*, int>> items;
	int drop = 0;
	std::string goName = "GO Name";
	std::string itemName = "";
	unsigned uid = 0;
	math::float3 scale;
	std::vector<std::string> itemList;
	std::vector<std::pair<std::string, int>> actLoot;

	GameObject* go;
	GameObject* list;
	Script* script = nullptr;

	void Expose(ImGuiContext * context);
	void GetItems();
	void Serialize(JSON_value * json) const;
	void DeSerialize(JSON_value * json);
};

#endif __EnemyLoot_h__
