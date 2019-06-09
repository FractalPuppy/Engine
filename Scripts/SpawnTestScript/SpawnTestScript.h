#ifndef  __SpawnTestScript_h__
#define  __SpawnTestScript_h__

#include "BaseScript.h"
#include <string>

#ifdef SpawnTestScript_EXPORTS
#define SpawnTestScript_API __declspec(dllexport)
#else
#define SpawnTestScript_API __declspec(dllimport)
#endif

class SpawnTestScript_API SpawnTestScript : public Script
{
	void Start() override;
	void Expose(ImGuiContext* context) override;

private:
	std::string prefab;
};

#endif __SpawnTestScript_h__
