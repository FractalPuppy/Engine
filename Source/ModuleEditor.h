#ifndef __ModuleEditor_h__
#define __ModuleEditor_h__

#include <list>
#include "Module.h"
#include "SDL_events.h"

class Panel;
class PanelConsole;
class PanelInspector;
class PanelConfiguration;
class PanelAbout;
class PanelHardware;
class PanelHierarchy;
class PanelTime;
class GameObject;
class MaterialEditor;
class FileExplorer;

class ModuleEditor :
	public Module
{
public:
	ModuleEditor();
	~ModuleEditor();
	bool Init(JSON * config) override;
	update_status PreUpdate() override;
	update_status Update(float dt) override;
	bool CleanUp() override;

	void RenderGUI() const;
	void ShowInspector();

	void processInput(SDL_Event* event) const;
	void AddFpsLog(float dt) const;
	void AddLog(const char *log) const;

private:
	void CreateDockSpace() const;
	void DrawPanels();
	void WindowsMenu();
	void HelpMenu();

public:
	PanelConsole *console = nullptr;
	PanelConfiguration *configuration = nullptr;
	PanelInspector *inspector = nullptr;
	PanelAbout *about = nullptr;
	PanelHardware *hardware = nullptr;
	PanelHierarchy *hierarchy = nullptr;
	PanelTime *time = nullptr;

	MaterialEditor *materialEditor = nullptr;
	FileExplorer *fileExplorer = nullptr;

private:
	std::list<Panel*> panels;
};

#endif //__ModuleEditor_h__
