#ifndef __ModuleScript_h__
#define __ModuleScript_h__

#include "Module.h"
#include <map>

class Script;
class ComponentScript;

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

	Script* GetScript(const ComponentScript& component, const std::string& name);
	void RemoveScript(const ComponentScript& component, const std::string& name);
private:
	void CheckScripts();
	std::string GetLastErrorAsString();

public:
	std::map<std::string, int> scripts; //name, modification date
	update_status status = UPDATE_CONTINUE;
	bool onStart = true;
private:
	std::list <const ComponentScript*> componentsScript;
	std::map<std::string, std::pair<HINSTANCE, int>> loadedDLLs; // name, dll, instances
	
};

#endif __ModuleScript_h__