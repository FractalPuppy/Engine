#ifndef __PLAYERSTATEWALKTOPICKITEM_H_
#define __PLAYERSTATEWALKTOPICKITEM_H_

#include "PlayerState.h"

class PlayerStateWalkToPickItem :	public PlayerState
{
public:
	PlayerStateWalkToPickItem(PlayerMovement* PM, const char* trigger);
	~PlayerStateWalkToPickItem();

	void Update() override;
	void CheckInput() override;
	void Enter() override;

public:
	float duration = 1.5f;
	std::vector<float3>path;
	unsigned pathIndex = 0u;
	GameObject* dustParticles = nullptr;

private:
	//functions
private:
	float moveTimer = 0.0f;
	float defaultMaxDist = 10000.f;
	float3 itemPosition = float3(0.f, 0.f, 0.f);
};

#endif // __PLAYERSTATEWALKTOPICKITEM_H_