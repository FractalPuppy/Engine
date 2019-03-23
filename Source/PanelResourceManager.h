#ifndef __PanelResourceManager_h__
#define __PanelResourceManager_h__

#include "Panel.h"
#include <vector>

class Resource;
class ResourceTexture;
struct ImVec2;

enum class SORTING { NONE, UID, FILE, EXPORTED, TYPE, REFERENCES};

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
	void DrawResourceMesh();
	void DrawResourceMaterial();
	void DrawResourceSkybox();
	void CleanUp();

private:
	Resource* auxResource = nullptr;
	Resource* previous = nullptr;

	int auxReferences = 0u;

	bool openEditor = false;
	bool openTextureWindow = false;
	bool openMeshWindow = false;
	bool openMaterialWindow = false;
	bool openSkyboxWindow = false;

	SORTING sortList = SORTING::REFERENCES;
	bool descending = false;
	std::vector<Resource*> resourcesList;
};

#endif __PanelResourceManager_h__