#ifndef __ModuleScript_h__
#define __ModuleScript_h__

#include "Module.h"
#include <map>
#include <set>
#include <vector>
#include <thread>

class Script;
class JSON_value;
class GameObject;

#ifndef _WINDEF_
struct HINSTANCE__; // Forward or never
typedef HINSTANCE__* HINSTANCE;
#endif

class ModuleScript :
	public Module
{
public:

	ModuleScript();
	~ModuleScript();

	bool Init(JSON* config) override;
	void SaveConfig(JSON* config) override;
	bool CleanUp() override;

	update_status Update(float dt) override;

	void DrawGUI() override;
	void LoadFromMemory(int resource);

	void AddScriptReference(Script* script, const std::string & name);

	Script* GetScript(const std::string& name);
	bool RemoveScript(Script* script, const std::string& name);
	bool RemoveDLL(const std::string & name, bool ignoreInMemory = false);

private:
	void Monitorize();
	void MonitorizeScripts();
	void UpdateScript();
	void ReloadAll();
	bool HotSwap(std::string scriptName);
	void HotReload(std::string scriptName, bool initialize = true);
	void InitializeScript(Script* script);
	void SaveScript(Script* script);
	void ResetScriptFlags();
	void ManageStartAndStopCursorIcon();

	std::string GetLastErrorAsString();

public:
	std::map<std::string, int> scripts; //name, modification date
	update_status status = UPDATE_CONTINUE;
	bool onStart = true;
private:
	std::vector <Script*> componentsScript;
	std::map<std::string, std::pair<HINSTANCE, int>> loadedDLLs; // name, dll, instances
	std::set<std::string>dllRemoveList;

	typedef std::pair<JSON_value*, GameObject*> scriptProperties;
	std::map<std::string, std::vector<scriptProperties>> scriptInfo;
	JSON* hotJson = nullptr;
	bool hotReloading = true;

	std::set<std::string> scriptsToReload;
	std::thread monitorThread;
	bool monitorizing = true;
	bool threadIsWorking = false;

	bool changeStartCursorIcon = true;
	bool changeStopCursorIcon = true;
};

#endif __ModuleScript_h__