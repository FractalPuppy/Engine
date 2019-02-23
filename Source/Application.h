#ifndef __Application_h__
#define __Application_h__

#include<list>
#include "Globals.h"
#include "Timer.h"

#define CONFIG_FILE "config.json"

class Module;
class ModuleRender;
class ModuleWindow;
class ModuleTextures;
class ModuleInput;
class ModuleCamera;
class ModuleProgram;
class ModuleEditor;
class ModuleScene;
class ModuleDebugDraw;
class ModuleFileSystem;
class ModuleResourceManager;
class ModuleTime;
class ModuleSpacePartitioning;

class Application
{
public:

	Application();
	~Application();

	bool Init();
	update_status Update();
	bool CleanUp();

	void SetTimer();

public:
	ModuleRender* renderer = nullptr;
	ModuleWindow* window = nullptr;
	ModuleTextures* textures = nullptr;
	ModuleInput* input = nullptr;
    ModuleCamera* camera = nullptr;
	ModuleProgram* program = nullptr;
	ModuleEditor* editor = nullptr;
	ModuleScene* scene = nullptr;
	ModuleDebugDraw* debug = nullptr;
	ModuleFileSystem* fsystem = nullptr;
	ModuleResourceManager* resManager = nullptr;
	ModuleTime* time = nullptr;
	ModuleSpacePartitioning* spacePartitioning = nullptr;

private:
	std::list<Module*> modules;
	Timer ms_timer;
	float dt;

};

extern Application* App;

#endif //__Application_h__