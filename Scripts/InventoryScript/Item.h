#ifndef  __Item_h__
#define  __Item_h__

#include <string>

enum class ItemType
{
	NONE = 0,
	QUICK,
	KEY,
	MATERIAL,
	WEAPON,
	HELMET,
	CHEST,
	PANTS,
	BOOTS,
	AMULET,
	RING
};

class Item
{

public:
	std::string name;
	std::string description;
	std::string sprite;
	ItemType type = ItemType::NONE;
	//3D Material/Prefab/Mesh 

};

#endif