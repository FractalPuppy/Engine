#ifndef __PanelResourceManager_h__
#define __PanelResourceManager_h__

#include "Panel.h"
#include <vector>

class Resource;
class ResourceTexture;
struct ImVec2;

enum class SORTING { NONE, UID, NAME, FILE, EXPORTED, TYPE, REFERENCES};

class PanelResourceManager : 
	public Panel
{
public:
	PanelResourceManager();
	~PanelResourceManager();

	void Draw() override;

private:
	void UpdateResourcesList();
	void OpenResourceEditor();
	void DrawResourceTexture();
	void DrawResourceModel();
	void DrawResourceMesh();
	void DrawResourceScene();
	void DrawResourceMaterial();
	void DrawResourceAnimation();
	void DrawResourceSkybox();
	void DrawResourceSM();
	void CleanUp();

private:
	Resource* auxResource = nullptr;
	Resource* previous = nullptr;

	int auxUID = 0;
	int auxReferences = 0;

	bool openEditor = false;
	bool openResourceWindow = false;

	SORTING sortList = SORTING::REFERENCES;
	bool descending = false;
	std::vector<Resource*> resourcesList;
};

#endif __PanelResourceManager_h__