#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleRender.h"
#include "ModuleTextures.h"
#include "ModuleInput.h"
#include "ModuleCamera.h"
#include "ModuleProgram.h"
#include "ModuleEditor.h"
#include "ModuleScene.h"
#include "ModuleDebugDraw.h"
#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h"
#include "ModuleTime.h"
#include "ModuleSpacePartitioning.h"
#include "ModuleScript.h"

#include "Timer.h"
#include "JSON.h"
#include "Brofiler.h"

using namespace std;

Application::Application()
{
	// Order matters: they will Init/start/update in this order
	modules.push_back(window = new ModuleWindow());
	modules.push_back(fsystem = new ModuleFileSystem());
	modules.push_back(input = new ModuleInput());
	modules.push_back(scripting = new ModuleScript());
	modules.push_back(renderer = new ModuleRender());
    modules.push_back(camera = new ModuleCamera());
	modules.push_back(textures = new ModuleTextures());
	modules.push_back(program = new ModuleProgram());
	modules.push_back(editor = new ModuleEditor());
	modules.push_back(debug = new ModuleDebugDraw());
	modules.push_back(scene = new ModuleScene());
	modules.push_back(resManager = new ModuleResourceManager());
	modules.push_back(time = new ModuleTime());
	modules.push_back(spacePartitioning = new ModuleSpacePartitioning());
}

Application::~Application()
{
	for(list<Module*>::iterator it = modules.begin(); it != modules.end(); ++it)
    {
        RELEASE(*it);
    }
}

bool Application::Init()
{
	Timer t;
	t.Start();

	bool ret = true;

	//Load config
	char* data = nullptr;
	fsystem->Load(CONFIG_FILE, &data);
	JSON* json = nullptr;
	if (data != nullptr)
	{
		json = new JSON(data);
	}

	//Init
	for (list<Module*>::iterator it = modules.begin(); it != modules.end() && ret; ++it)
		ret = (*it)->Init(json);

	if (data != nullptr)
	{
		RELEASE_ARRAY(data);
		RELEASE(json);
	}

	//Start
	for (list<Module*>::iterator it = modules.begin(); it != modules.end() && ret; ++it)
		ret = (*it)->Start();

	LOG("Init + Start time: %d ms",t.Stop());
	ms_timer.Start();

	return ret;
}

update_status Application::Update()
{
	PROFILE;

	SetTimer();
	update_status ret = UPDATE_CONTINUE;

	for(list<Module*>::iterator it = modules.begin(); it != modules.end() && ret == UPDATE_CONTINUE; ++it)
		ret = (*it)->PreUpdate();

	for(list<Module*>::iterator it = modules.begin(); it != modules.end() && ret == UPDATE_CONTINUE; ++it)
		ret = (*it)->Update(dt);

	for(list<Module*>::iterator it = modules.begin(); it != modules.end() && ret == UPDATE_CONTINUE; ++it)
		ret = (*it)->PostUpdate();

	editor->AddFpsLog(dt);
	return ret;
}

bool Application::CleanUp()
{
	bool ret = true;

	//Save config
	JSON *json = new JSON();
	for (list<Module*>::iterator it = modules.begin(); it != modules.end() && ret == UPDATE_CONTINUE; ++it)
		(*it)->SaveConfig(json);
	fsystem->Save(CONFIG_FILE, json->ToString().c_str(), json->Size());
	RELEASE(json);
	//CleanUp
	for (list<Module*>::reverse_iterator it = modules.rbegin(); it != modules.rend() && ret; ++it)
	{
		ret = (*it)->CleanUp();
	}
	return ret;
}

void Application::SetTimer()
{
	dt = (float)ms_timer.Read() / 1000.f;
	ms_timer.Start();
}
