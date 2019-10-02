#include "ModuleCollisions.h"

#include "ComponentBoxTrigger.h"
#include "Geometry/OBB.h"

update_status ModuleCollisions::Update(float dt)
{
	for (auto player : playerHpBoxes)
	{
		if (!player->enabled) continue;

		const math::OBB* player_obb = player->GetOBB();
		for (auto enemyAttack : enemyAttackBoxes)
		{
			if (!enemyAttack->enabled) continue;

			const math::OBB* enemyAttack_obb = enemyAttack->GetOBB();
			if (player_obb->Intersects(*enemyAttack_obb))
			{
				player->AddOverlap(enemyAttack);
				enemyAttack->AddOverlap(player);
			}
		}

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

	for (auto enemyHp : enemyHpBoxes)
	{
		if (!enemyHp->enabled) continue;

		const math::OBB* enemyHp_obb = enemyHp->GetOBB();
		for (auto playerAttack : playerAttackBoxes)
		{
			if (!playerAttack->enabled) continue;

			const math::OBB* playerAttack_obb = playerAttack->GetOBB();
			if (enemyHp_obb->Intersects(*playerAttack_obb))
			{
				enemyHp->AddOverlap(playerAttack);
				playerAttack->AddOverlap(enemyHp);
			}
		}
	}

	return update_status::UPDATE_CONTINUE;
}

void ModuleCollisions::AddBox(ComponentBoxTrigger* box, BoxTriggerType boxType)
{
	RemoveBox(box);

	switch (boxType)
	{
		case BoxTriggerType::Unknown:
			break;
		case BoxTriggerType::PlayerHp:
			playerHpBoxes.push_back(box);
			break;
		case BoxTriggerType::PlayerAttack:
			playerAttackBoxes.push_back(box);
			break;
		case BoxTriggerType::EnemyHp:
			enemyHpBoxes.push_back(box);
			break;
		case BoxTriggerType::EnemyAttack:
			enemyAttackBoxes.push_back(box);
			break;
		case BoxTriggerType::Other:
			otherBoxes.push_back(box);
			break;
	}
}

bool ModuleCollisions::RemoveBox(ComponentBoxTrigger* box)
{
	switch (box->GetBoxTriggerType())
	{
		case BoxTriggerType::PlayerHp:
			for (int i = playerHpBoxes.size() - 1; i >= 0; --i)
			{
				if (playerHpBoxes[i] == box)
				{
					playerHpBoxes.erase(playerHpBoxes.begin() + i);
					return true;
				}
			}
			break;
		case BoxTriggerType::PlayerAttack:
			for (int i = playerAttackBoxes.size() - 1; i >= 0; --i)
			{
				if (playerAttackBoxes[i] == box)
				{
					playerAttackBoxes.erase(playerAttackBoxes.begin() + i);
					return true;
				}
			}
			break;
		case BoxTriggerType::EnemyHp:
			for (int i = enemyHpBoxes.size() - 1; i >= 0; --i)
			{
				if (enemyHpBoxes[i] == box)
				{
					enemyHpBoxes.erase(enemyHpBoxes.begin() + i);
					return true;
				}
			}
			break;
		case BoxTriggerType::EnemyAttack:
			for (int i = enemyAttackBoxes.size() - 1; i >= 0; --i)
			{
				if (enemyAttackBoxes[i] == box)
				{
					enemyAttackBoxes.erase(enemyAttackBoxes.begin() + i);
					return true;
				}
			}
			break;
		case BoxTriggerType::Other:
			for (int i = otherBoxes.size() - 1; i >= 0; --i)
			{
				if (otherBoxes[i] == box)
				{
					otherBoxes.erase(otherBoxes.begin() + i);
					return true;
				}
			}
			break;
	}

	return false;
}
