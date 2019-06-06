#ifndef __Module_Collisions_h__
#define __Module_Collisions_h__

#include "Module.h"
#include <vector>

class ComponentBoxTrigger;

struct OverlapPair
{
	OverlapPair(ComponentBoxTrigger* player, ComponentBoxTrigger* other) : player(player), other(other) {}

	ComponentBoxTrigger* player = nullptr;
	ComponentBoxTrigger* other  = nullptr;
};

class ModuleCollisions : public Module
{
public:
	virtual update_status Update(float dt) override;

	void AddBox(ComponentBoxTrigger* box, bool isPlayer);
	bool RemoveBox(ComponentBoxTrigger* box);

private:
	std::vector<ComponentBoxTrigger*> playerBoxes;
	std::vector<ComponentBoxTrigger*> otherBoxes;
};

#endif // !__Module_Collisions_h__

