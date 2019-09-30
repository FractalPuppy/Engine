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
	void Expose(ImGuiContext* context);

	void Serialize(JSON_value* json) const;
	void DeSerialize(JSON_value* json);

public:
	void DropItems();

	std::vector<std::string> itemList;
	math::float3 rotationOffset = math::float3(-90.0f, 0.0f, 0.0f);

private:
	GameObject* itemListGO = nullptr;
};

#endif __LootDropScript_h__
