#ifndef __PanelBrowser_h__
#define __PanelBrowser_h__

#include "Panel.h"

#include "Math\float2.h"
#include <string>
#include <vector>
#include <stack>

#define MAX_FILENAME 30

class Resource;
class ResourceTexture;
class GameObject;

class PanelBrowser :
	public Panel
{
public:
	PanelBrowser();
	~PanelBrowser();

	bool Init() override;
	void Draw();
	inline void ResetDragNDrop()
	{
		dragAsset = nullptr;
	}

private:
	void DrawFolderIcon(const char* dir, int itemNumber);
	void DrawFileIcon(const char* file, int itemNumber);

	void DrawBrowserContextMenu();
	void DrawFileContextMenu();
	void DrawFolderContextMenu();
	void DrawImportConfigurationPopUp();
	void DrawRenameFilePopUp();
	void DrawRenameFolderPopUp();
	void DrawNewFolderPopUp();

public:
	bool folderContentDirty = true;

private:
	std::string path;
	std::stack<std::string> pathStack;
	std::vector<std::string> files;			// List of the files in current path
	std::vector<std::string> dirs;			// List of the folders in current path

	bool openImportConfigPopUp = false;
	bool openRenameFilePopUp = false;
	bool openRenameFolderPopUp = false;
	bool openNewFolderPopUp = false;

	Resource* fileSelected = nullptr;
	std::string folderSelected = "";

	// Rename variables
	char newFileName[MAX_FILENAME] = "";	// Aux string for rename file
	char newFolderName[MAX_FILENAME] = "";	// Aux string for rename folder
	bool invalidName = false;

	// Icons
	ResourceTexture* folderIcon = nullptr;	
	ResourceTexture* fileIcon = nullptr;
	ResourceTexture* fbxIcon = nullptr;
	ResourceTexture* pngIcon = nullptr;
	ResourceTexture* jpgIcon = nullptr;
	ResourceTexture* tgaIcon = nullptr;
	ResourceTexture* tifIcon = nullptr;
	ResourceTexture* ddsIcon = nullptr;
	ResourceTexture* m4tIcon = nullptr;
	ResourceTexture* jsonIcon = nullptr;
	ResourceTexture* sc3neIcon = nullptr;
	ResourceTexture* animati0nIcon = nullptr;
	ResourceTexture* st4tem4chineIcon = nullptr;
	ResourceTexture* pr3fabIcon = nullptr;

	//Drag N Drop
	GameObject* dragAsset = nullptr;
};

#endif //__PanelBrowser_h__