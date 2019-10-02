#ifndef  __GodMode_h__
#define  __GodMode_h__

#include "BaseScript.h"

#ifdef GodMode_EXPORTS
#define GodMode_API __declspec(dllexport)
#else
#define GodMode_API __declspec(dllimport)
#endif

class GodMode_API GodMode : public Script
{
public:

	void Start() override;
	void Update() override;
	void Expose(ImGuiContext* context) override;
	void ClickedGodMode();
	void SwitchLevel() const;
	void AddExperience() const;
	void InfiniteHealth() const;
	void InfiniteMana() const;
	void MaxStats() const;
	void Exposure() const;
	inline virtual GodMode* Clone() const
	{
		return new GodMode(*this);
	}

private:
	bool isGod = false;
	bool wasGod = false;
	GameObject* playerGO = nullptr;
	GameObject* gameLoopGO = nullptr;
	GameObject* xpGO = nullptr;
};

extern "C" GodMode_API Script* CreateScript();
#endif __GodMode_h__
