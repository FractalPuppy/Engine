#ifndef __Module_Collisions_h__
#define __Module_Collisions_h__

#include "Module.h"
#include <vector>

class ComponentBoxTrigger;
enum class BoxTriggerType;

struct OverlapPair
{
	OverlapPair(ComponentBoxTrigger* first, ComponentBoxTrigger* second) : first(first), second(second) {}

	ComponentBoxTrigger* first = nullptr;
	ComponentBoxTrigger* second  = nullptr;
};

class ModuleCollisions : public Module
{
public:
	virtual update_status Update(float dt) override;

	void AddBox(ComponentBoxTrigger* box, BoxTriggerType isPlayer);
	bool RemoveBox(ComponentBoxTrigger* box);

private:
	std::vector<ComponentBoxTrigger*> playerHpBoxes;
	std::vector<ComponentBoxTrigger*> playerAttackBoxes;
	std::vector<ComponentBoxTrigger*> enemyHpBoxes;
	std::vector<ComponentBoxTrigger*> enemyAttackBoxes;
	std::vector<ComponentBoxTrigger*> otherBoxes;
};

#endif // !__Module_Collisions_h__

