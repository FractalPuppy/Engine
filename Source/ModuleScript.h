#ifndef __ModuleScript_h__
#define __ModuleScript_h__

#include "Module.h"
#include <map>

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

	bool Start() override;
	update_status Update(float dt) override;

	void LoadFromMemory(int resource);

	Script* AddScript(const std::string& script);
	void RemoveScript(const std::string& name, Script* script);

private:
	void CheckScripts();

public:
	std::map<std::string, int> scripts; //name, modification date
private:
	std::list<Script*> scriptInstances;
	std::map<std::string, std::pair<HINSTANCE, int>> loadedDLLs; // name, dll, instances
	
	bool onStart = true;
};

#endif __ModuleScript_h__