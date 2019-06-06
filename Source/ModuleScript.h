#ifndef __ModuleScript_h__
#define __ModuleScript_h__

#include "Module.h"
#include <map>
#include <vector>

class Script;

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
	update_status Update(float dt) override;

	void LoadFromMemory(int resource);

	void AddScriptReference(Script* script, const std::string & name);

	Script* GetScript(const std::string& name);
	bool RemoveScript(Script* script, const std::string& name);
	bool RemoveDLL(const std::string & name);

private:
	void CheckScripts();
	void ResetScriptFlags();

	std::string GetLastErrorAsString();

public:
	std::map<std::string, int> scripts; //name, modification date
	update_status status = UPDATE_CONTINUE;
	bool onStart = true;
private:
	std::list <Script*> componentsScript;
	std::map<std::string, std::pair<HINSTANCE, int>> loadedDLLs; // name, dll, instances
	std::vector<std::string>dllRemoveList;
};

#endif __ModuleScript_h__