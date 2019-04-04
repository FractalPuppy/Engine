#ifndef __MODULENAVIGATION_H__
#define __MODULENAVIGATION_H__

#include "Module.h"
#include <vector>
//<>
class GameObject;
class ModuleNavigation :
	public Module
{
public:
	ModuleNavigation();
	~ModuleNavigation();

	void DrawGUI()override;
	void navigableObjectToggled(GameObject* obj, const bool newState);

	//variables
	std::vector<GameObject*> navigationMeshes;
	std::vector<GameObject*> agents;
	std::vector<GameObject*> obstacles;

private:
	void removeNavMesh(unsigned ID);
	void generateNavigability();

	//variables
	float maxRadius = 5.0f;
	float maxHeight = 5.0f;
	float maxSlopeScaling = 20.0f;
	float maxStepHeightScaling = 5.0f;
	
	char newCharacter[64] = "New Character";
	float characterMaxRadius = 5.0f;
	float characterMaxHeight = 5.0f;
	float characterMaxSlopeScaling = 20.0f;
	float characterMaxStepHeightScaling = 5.0f;
	
	const float sliderIncreaseSpeed = 0.03f;
	const float minSliderValue = 0.01f;
	const float maxSliderValue = 100.0f;
	const float maxSlopeValue = 60.0f;

	//navigation mesh properties
	
};

#endif