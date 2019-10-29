#ifndef  __LootDropScript_h__
#define  __LootDropScript_h__

#include "BaseScript.h"
#include <vector>

#include "Math\float3.h"

class ImGuiContext;

#ifdef LootDropScript_EXPORTS
#define LootDropScript_API __declspec(dllexport)
#else
#define LootDropScript_API __declspec(dllimport)
#endif

class LootDropScript_API LootDropScript : public Script
{
public:
	inline virtual LootDropScript* Clone() const
	{
		return new LootDropScript(*this);
	}

private:
	void Start() override;
	void Update() override;
	void Expose(ImGuiContext* context);

	void Serialize(JSON_value* json) const;
	void DeSerialize(JSON_value* json);

public:
	void DropItems();
	void DropItemsInCircle(float radius);			// Sapwns Items following a circle
	void DropItemsInSemiCircle(float radius);		// Sapwns Items following a semicircle

	std::vector<std::string> itemList;

	// Items are spawned as children of itemList GO: the following variables are to set an offset from that GO position and rotation 
	math::float3 positionOffset = math::float3(0.0f, 0.0f, 0.0f);
	math::float3 rotationOffset = math::float3(-90.0f, 0.0f, 0.0f);

	// Animation variables
	bool animate = true;						// Spawn the items with animation?
	float animDuration = 0.8f;					// How much time the animation lasts
	float animHigh = 200.0f;					// How high the items will jump on spawn animation

private:
	GameObject* itemListGO = nullptr;			// Game Object were all items are set as a child
	std::vector<GameObject*> spawnedGOList;		// Vector with pointers to all spawned items
	std::vector<math::float3> itemsPosTarget;	// Vector with difference between target position and start position for each spawned item
	bool spawned = false;						// Have the items been spawned?

	float timer = 0.0f;							// Timer for the animation
};

#endif __LootDropScript_h__
