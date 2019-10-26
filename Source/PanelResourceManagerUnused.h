#ifndef __PanelResourceManagerUnused_h__
#define __PanelResourceManagerUnused_h__

#include "PanelResourceManager.h"

#include "Panel.h"
#include <vector>

class Resource;
class ResourceTexture;
struct ImVec2;

class PanelResourceManagerUnused :
	public Panel
{
public:
	PanelResourceManagerUnused();
	~PanelResourceManagerUnused();

	void Draw() override;

private:
	void UpdateResourcesList();

	void OpenResourceEditor();

	void DrawResourceTexture();
	void DrawResourceModel();
	void DrawResourceMesh();
	void DrawResourceAudio();
	void DrawResourceScene();
	void DrawResourceMaterial();
	void DrawResourceAnimation();
	void DrawResourceSkybox();
	void DrawResourceSM();
	void DrawResourcePrefab();

	void DrawFilterByResourceMenu();

	void CleanUp();

private:
	Resource* auxResource = nullptr;
	Resource* previous = nullptr;

	int auxUID = 0;
	int auxReferences = 0;

	bool openEditor = false;
	bool openResourceWindow = false;

	// Sorting
	SORTING sortList = SORTING::REFERENCES;
	bool descending = false;

	// Filtering
	RESOURCE_FILTER filterByResource = RESOURCE_FILTER::NONE;
	REFERENCE_FILTER filterByReferenceCount = REFERENCE_FILTER::NONE;

	std::vector<Resource*> resourcesList;

	bool autoRefresh = false;
};

#endif __PanelResourceManagerUnused_h__