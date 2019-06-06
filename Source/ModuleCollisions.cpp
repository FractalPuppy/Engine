#include "ModuleCollisions.h"

#include "ComponentBoxTrigger.h"
#include "Geometry/OBB.h"

update_status ModuleCollisions::Update(float dt)
{
	for (auto player : playerBoxes)
	{
		if (!player->enabled) continue;

		const math::OBB* player_obb = player->GetOBB();
		for (auto other : otherBoxes)
		{
			if (!other->enabled) continue;

			const math::OBB* other_obb = other->GetOBB();
			if (player_obb->Intersects(*other_obb))
			{
				player->AddOverlap(other);
				other->AddOverlap(player);
			}
		}
	}

	return update_status::UPDATE_CONTINUE;
}

void ModuleCollisions::AddBox(ComponentBoxTrigger* box, bool isPlayer)
{
	RemoveBox(box);
	
	if (isPlayer) playerBoxes.push_back(box);
	else otherBoxes.push_back(box);
}

bool ModuleCollisions::RemoveBox(ComponentBoxTrigger* box)
{
	for (int i = playerBoxes.size() - 1; i >= 0; --i)
	{
		if (playerBoxes[i] == box)
		{
			playerBoxes.erase(playerBoxes.begin() + i);
			return true;
		}
	}
	for (int i = otherBoxes.size() - 1; i >= 0; --i)
	{
		if (otherBoxes[i] == box)
		{
			otherBoxes.erase(otherBoxes.begin() + i);
			return true;
		}
	}

	return false;
}
