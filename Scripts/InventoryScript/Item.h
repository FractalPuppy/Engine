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