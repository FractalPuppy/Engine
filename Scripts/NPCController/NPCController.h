#ifndef  __NPCController_h__
#define  __NPCController_h__

#include "BaseScript.h"

#include "Geometry/AABB.h"

#ifdef NPCController_EXPORTS
#define NPCController_API __declspec(dllexport)
#else
#define NPCController_API __declspec(dllimport)
#endif

class NPCController_API NPCController : public Script
{
	void Start() override;
	void Update() override;

	void Expose(ImGuiContext* context);

	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;

	inline virtual NPCController* Clone() const
	{
		return new NPCController(*this);
	}
private:
	GameObject* player = nullptr;
	std::string playerName = "Player";
	std::string playerBboxName = "PlayerMesh";
	std::string myBboxName = "NPCMesh";

	// BBoxes
	math::AABB* myBbox = nullptr;
	math::AABB* playerBbox = nullptr;

	// Canvas elements
	GameObject* dialogueGO = nullptr;
	std::string dialogueGOName = "DialogueCanvas";

	//HUD
	GameObject* hudGO = nullptr;
};

extern "C" NPCController_API Script* CreateScript();

#endif __NPCController_h__
