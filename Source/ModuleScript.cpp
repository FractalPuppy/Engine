#include "Application.h"

#include "ModuleScript.h"
#include "ModuleFileSystem.h"
#include "ModuleTime.h"
#include "ModuleUI.h"
#include "MouseController.h"

#include "GameObject.h"
#include "Component.h"

#include "BaseScript.h"
#include "MemoryModule.h"
#include "imgui.h"
#include "PlayerPrefs.h"
#include <assert.h>
#include <windows.h>
#include <iostream>
#include "JSON.h"
#include "SDL_timer.h"

#define MONITORIZE_TIME 1000
typedef Script*(__cdecl *CreatePointer)();

ModuleScript::ModuleScript()
{
}

ModuleScript::~ModuleScript()
{

	for (const auto& dll : loadedDLLs)
	{
		if (!FreeLibrary(dll.second.first))
		{
			LOG("CAN'T RELEASE %s", dll.first);
		}
	}
}

bool ModuleScript::Init(JSON* config)
{
	PlayerPrefs::Load();
	SetDllDirectory(SCRIPTS);

	MonitorizeScripts();

	JSON_value* scriptJson = config->GetValue("scripts");
	if (scriptJson != nullptr)
	{
		hotReloading = scriptJson->GetInt("hotReloading", hotReloading);
	}

#ifndef GAME_BUILD
	if (hotReloading)
	{
		monitorThread = std::thread(&ModuleScript::Monitorize, this);
		monitorThread.detach();
	}
#endif // !GAME_BUILD

	//LoadFromMemory(IDR_DLL1);
	return true;
}

void ModuleScript::SaveConfig(JSON* config)
{
	JSON_value* scriptJson = config->CreateValue();

	scriptJson->AddInt("hotReloading", hotReloading);
	config->AddValue("scripts", *scriptJson);
}

bool ModuleScript::CleanUp()
{
#ifndef GAME_BUILD
	monitorizing = false;
	while (threadIsWorking)
	{
		SDL_Delay(100);
	}
#endif // !GAME_BUILD
	PlayerPrefs::DeleteAll(true); //Deletes All memory allocated and prefs files
	return true;
}

update_status ModuleScript::Update(float dt)
{
	if (dllRemoveList.size() > 0)
	{
		std::set<std::string>::iterator it;
		while (!dllRemoveList.empty())
		{
			it = dllRemoveList.begin();
			while (it != dllRemoveList.end())
			{
				if (RemoveDLL(*it, true))
				{
					dllRemoveList.erase(it++);
				}
				else
				{
					++it;
				}
			}
		}
	}
	if (!scriptsToReload.empty())
	{
		UpdateScript();
		scriptsToReload.clear();
	}
	if (App->time->gameState == GameState::RUN)
	{
		if (onStart)
		{
			for (size_t i = 0; i < componentsScript.size(); i++)
			{
				if (componentsScript[i]->gameobject->isActive())
				{
					componentsScript[i]->Awake();
					componentsScript[i]->hasBeenAwoken = true;
				}
			}

			for (size_t i = 0; i < componentsScript.size(); i++)
			{
				if (componentsScript[i]->enabled)
				{
					componentsScript[i]->Start();
					componentsScript[i]->hasBeenStarted = true;
					componentsScript[i]->SetGameStandarCursor(App->ui->gameStandarCursor);
				}
			}
		}
		for (size_t i = 0; i < componentsScript.size(); i++)
		{
			if (componentsScript[i]->enabled)
			{
				componentsScript[i]->Update();
			}
		}
	}

	if (!onStart && App->time->gameState == GameState::STOP)
	{
		ResetScriptFlags();
	}

	ManageStartAndStopCursorIcon();

	onStart = App->time->gameState == GameState::STOP;
	return status;
}

void ModuleScript::ResetScriptFlags()
{
	for (const auto& script : componentsScript)
	{
		script->hasBeenAwoken = false;
		script->hasBeenStarted = false;
	}
}

void ModuleScript::DrawGUI()
{
	ImGui::Checkbox("Hot Reloading", &hotReloading);
}

void ModuleScript::LoadFromMemory(int resource) //TODO: Load from memory in shipping build
{
	HRSRC hResource = FindResourceA(nullptr, MAKEINTRESOURCEA(resource), "DLL");
	HGLOBAL hMemory = LoadResource(nullptr, hResource);

	assert(hMemory != nullptr);
	int size_bytes = SizeofResource(nullptr, hResource);
	void* ptr = LockResource(hMemory);
	assert(ptr != nullptr);
	HMEMORYMODULE dll = MemoryLoadLibrary(ptr);
	if (dll != nullptr)
	{
		CreatePointer Create = (CreatePointer)MemoryGetProcAddress(dll, "CreateScript");
		if (Create != nullptr)
		{
			Script* script = Create();
			//script->SetApp(App);
			//script->SetGameObject(App->scene->root);
			//scriptInstances.push_back(script);
			//script->Start();
			//script->Update();
		}
	}
}

void ModuleScript::AddScriptReference(Script* script, const std::string&name)
{
	std::map<std::string, std::pair<HINSTANCE, int>>::iterator itDll = loadedDLLs.find(name);
	if (itDll != loadedDLLs.end())
	{
		componentsScript.push_back(script);
		if (itDll->second.second == 0)
		{
			std::set<std::string>::iterator itList = dllRemoveList.find(name);
			if (itList != dllRemoveList.end())
			{
				dllRemoveList.erase(itList);
			}
		}
		itDll->second.second++;
	}
}

Script* ModuleScript::GetScript(const std::string& name)
{
	HINSTANCE dll;
	std::map<std::string, std::pair<HINSTANCE, int>>::iterator itDll = loadedDLLs.find(name);
	if (itDll != loadedDLLs.end())
	{
		dll = itDll->second.first;
		if (itDll->second.second == 0)
		{
			std::set<std::string>::iterator itList = dllRemoveList.find(name);
			if (itList != dllRemoveList.end())
			{
				dllRemoveList.erase(itList);
			}
		}
		itDll->second.second++;
	}
	else
	{
		dll = LoadLibrary((name + DLL).c_str());
		if (dll != nullptr)
		{
			loadedDLLs.insert(std::pair<std::string,
			std::pair<HINSTANCE, int>>(name, std::pair<HINSTANCE, int>(dll, 1)));
		}
		else
		{
			LOG(GetLastErrorAsString().c_str());
			return nullptr;
		}
	}
	assert(dll != nullptr);
	CreatePointer Create = (CreatePointer)GetProcAddress(dll, "CreateScript");
	assert(Create != nullptr);
	if (Create != nullptr)
	{
		Script* script = Create();
		script->SetApp(App);
		script->name = name;
		componentsScript.push_back(script);
		return script;
	}
	return nullptr;
}

bool ModuleScript::RemoveScript(Script* script, const std::string& name)
{
	std::map<std::string, std::pair<HINSTANCE,int>>::iterator itDll = loadedDLLs.find(name);
	if (itDll != loadedDLLs.end())
	{
		bool scriptRemoved = false;
		for (auto it = componentsScript.begin(); it != componentsScript.end(); ++it)
		{
			if (*it == script)
			{
				componentsScript.erase(it);
				scriptRemoved = true;
				break;
			}
		}
		if (!scriptRemoved) return false;

		itDll->second.second--;
		if(itDll->second.second == 0)
		{
			dllRemoveList.insert(name);
		}
	}
	else
	{
		LOG("DLL %s Not Found!", name);
		return false;
	}
	return true;
}

bool ModuleScript::HotSwap(std::string scriptName)
{
	if (!App->fsystem->Delete((SCRIPTS + scriptName + DLL).c_str())) return false; //Deleted old DLL
	if (!App->fsystem->Rename(SCRIPTS, (scriptName + HOT).c_str(), scriptName.c_str(), DLL)) return false; //Changed name new DLL
	HotReload(scriptName);
	return true;
}

void ModuleScript::HotReload(std::string scriptName, bool initialize) //TODO: unload properly all scripts on loading new scene
{
	HINSTANCE dll = LoadLibrary((scriptName + DLL).c_str());
	if (dll != nullptr)
	{
		loadedDLLs.insert(std::pair<std::string,
			std::pair<HINSTANCE, int>>(scriptName, std::pair<HINSTANCE, int>(dll, 1)));
	}

	assert(dll != nullptr);
	CreatePointer Create = (CreatePointer)GetProcAddress(dll, "CreateScript");
	assert(Create != nullptr);
	if (Create != nullptr)
	{
		auto scripts = scriptInfo.find(scriptName);
		if (scripts == scriptInfo.end()) return;

		for (const auto& info : scripts->second)
		{
			Script* script = Create();
			script->SetApp(App);
			script->name = scriptName;
			componentsScript.push_back(script);
			script->DeSerialize(info.first);
			script->SetGameObject(info.second);
			info.second->components.push_back((Component*)script);

			if (initialize)
			{
				InitializeScript(script);
			}
		}
	}
}

void ModuleScript::InitializeScript(Script* script)
{
	if (script->gameobject->isActive())
	{
		script->Awake();
		script->hasBeenAwoken = true;
	}

	if (script->enabled)
	{
		script->Start();
		script->hasBeenStarted = true;
	}
}

bool ModuleScript::RemoveDLL(const std::string& name, bool ignoreInMemory)
{
	std::map<std::string, std::pair<HINSTANCE, int>>::iterator itDll = loadedDLLs.find(name);
	if (itDll != loadedDLLs.end())
	{
		if (GetModuleHandle((name + DLL).c_str()) == 0)
		{
			LOG("DLL not even loaded");
			loadedDLLs.erase(itDll);
			return true;
		}
		if (!FreeLibrary(itDll->second.first))
		{
			LOG("CAN'T RELEASE %s", name);
			return false;
		}
		if (!ignoreInMemory && GetModuleHandle((name + DLL).c_str()) != 0)
		{
			LOG("DLL still in memory");
			return false;
		}
		else
		{
			loadedDLLs.erase(itDll);
		}
	}
	else
	{
		//LOG("DLL %s Not Found!", name); //FIXME: Not working with multithread fsystem
		return false;
	}
	return true;
}

void ModuleScript::Monitorize()
{
	while (monitorizing)
	{
		threadIsWorking = true;
		MonitorizeScripts();
		threadIsWorking = false;
		SDL_Delay(MONITORIZE_TIME);
	}
}

void ModuleScript::MonitorizeScripts() //HOT SWAP IF USED DLL
{
	std::vector<std::string> scriptFiles = App->fsystem->GetFolderContent(SCRIPTS, true);
	std::map<std::string, int>::iterator it;

	for (const auto& scriptFile : scriptFiles)
	{
		std::string scriptName = App->fsystem->GetFilename(scriptFile);

		int time = App->fsystem->GetModTime((SCRIPTS + scriptFile).c_str());
		it = scripts.find(scriptName);
		if (it != scripts.end())
		{
			if (time > it->second || App->fsystem->GetExtension(scriptFile) == HOT)
			{
				//Update DLL but only if already loaded!!
				it->second = time;
				LOG("Script %s updated", scriptName.c_str());
				if (App->fsystem->GetExtension(scriptFile) == HOT)
				{
					scriptsToReload.insert(scriptName);
				}
			}
		}
		else
		{
			scripts.insert(std::pair<std::string, int>(scriptName, time));
		}
	}
}

void ModuleScript::SaveScript(Script* script)
{
	std::string name = script->name;
	JSON_value* value = hotJson->CreateValue();
	script->Serialize(value);

	auto itInfo = scriptInfo.find(script->name);

	scriptProperties properties = std::make_pair(value, script->gameobject);
	if (itInfo != scriptInfo.end())
	{
		//add instance
		itInfo->second.emplace_back(properties);
	}
	else
	{
		std::vector<scriptProperties> info;
		info.emplace_back(properties);
		scriptInfo.insert(std::make_pair(script->name, info));
	}

	script->gameobject->RemoveComponent(*script);
	std::set<std::string>::iterator it = dllRemoveList.find(name);
	if (it != dllRemoveList.end())
	{
		dllRemoveList.erase(it);
	}
}

void ModuleScript::UpdateScript()
{
	hotJson = new JSON();
	for (unsigned i = 0u; i < componentsScript.size(); ++i)
	{
		if(scriptsToReload.find(componentsScript[i]->name) != scriptsToReload.end())
		{
			SaveScript(componentsScript[i]);
			--i;
		}
	}

	bool reloadFailed = false;
	for (auto scriptName : scriptsToReload)
	{
		if (!RemoveDLL(scriptName) || !HotSwap(scriptName))
		{
			reloadFailed = true;
		}
	}

	if(reloadFailed)//Reload All
	{
		for (unsigned i = 0u; i < componentsScript.size(); ++i)
		{
			if (scriptsToReload.find(componentsScript[i]->name) != scriptsToReload.end()) continue;
			SaveScript(componentsScript[i]);
			--i;
		}
		ReloadAll();
	}
	else
	{
		scriptInfo.clear();
	}

	RELEASE(hotJson);
}

void ModuleScript::ReloadAll()
{
	std::vector<std::string> dllNames;

	for (const auto dll : loadedDLLs) {
		dllNames.push_back(dll.first);
	}

	while (!loadedDLLs.empty())
	{
		for (const auto name : dllNames)
		{
			RemoveDLL(name);
		}
	}

	for (auto scriptName : scriptsToReload)
	{
		App->fsystem->Delete((SCRIPTS + scriptName + DLL).c_str()); //Deleted old DLL
		App->fsystem->Rename(SCRIPTS, (scriptName + HOT).c_str(), scriptName.c_str(), DLL); //Changed name new DLL
	}

	for (size_t i = 0; i < dllNames.size(); i++)
	{
		HotReload(dllNames[i], false);
	}
	scriptInfo.clear();

	for (size_t i = 0; i < componentsScript.size(); i++)
	{
		InitializeScript(componentsScript[i]);
	}
}

std::string ModuleScript::GetLastErrorAsString()
{
	//Get the error message, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return std::string(); //No error message has been recorded

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	std::string message(messageBuffer, size);

	//Free the buffer.
	LocalFree(messageBuffer);

	return message;
}

void ModuleScript::ManageStartAndStopCursorIcon()
{
	if (App->time->gameState == GameState::RUN && changeStartCursorIcon)
	{
		MouseController::ChangeCursorIcon(App->ui->gameStandarCursor);
		changeStartCursorIcon = false;
		changeStopCursorIcon = true;
	}
	else if (App->time->gameState == GameState::STOP && changeStopCursorIcon)
	{
		MouseController::ChangeWindowsCursorIcon();
		changeStartCursorIcon = true;
		changeStopCursorIcon = false;
	}
}