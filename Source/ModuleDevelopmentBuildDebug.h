#ifndef __ModuleDevelopmentBuildDebug_h__
#define __ModuleDevelopmentBuildDebug_h__

#include "Module.h"
#include "SDL_events.h"
#include <vector>

class ComponentCamera;

class ModuleDevelopmentBuildDebug:
	public Module
{
public:

	bool Init(JSON * config) override;
	update_status PreUpdate() override;
	update_status Update(float dt) override;
	bool CleanUp() override;

	void RenderGUI() const;

	void processInput(SDL_Event* event) const;

	unsigned frames = 0u;
	float totalTime = .0f;

	std::vector<float> current; //last second
	float currentFps = .0f;
	float currentFrameTime = .0f;
	float timer = 0.0f;
	ComponentCamera* godModeCamera = nullptr;
};

#endif //__ModuleEditor_h__
