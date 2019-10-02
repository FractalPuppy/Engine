#ifndef  __Item_h__
#define  __Item_h__

#include <string>

#include "PlayerMovement.h"

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

	Item() {};
	Item(const Item& item)		// Copy constructor
	{
		name = item.name;
		description = item.description;
		sprite = item.sprite;
		type = item.type;
		isEquipped = item.isEquipped;
		meshUID = item.meshUID;
		materialUID = item.materialUID;
		stats = item.stats;
	}

	inline bool isEqual(const Item& item) const {return this == &item;}
	friend bool operator==(const Item& lhs, const Item& rhs) { return lhs.isEqual(rhs); }

	std::string name;
	std::string description;
	std::string sprite;
	ItemType type = ItemType::NONE;
	bool isEquipped = false;
	//3D Material/Prefab/Mesh 
	unsigned meshUID = 0u;
	unsigned materialUID = 0u;

	PlayerStats stats = { 0.0f, 0.0f, 0U, 0U, 0.0f, 0.0f };
};

#endif