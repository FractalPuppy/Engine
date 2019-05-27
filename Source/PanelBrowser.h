#ifndef __PanelBrowser_h__
#define __PanelBrowser_h__

#include "Panel.h"
#include "ResourceTexture.h"

#include "MathGeoLib\include\Math\float2.h"
#include <string>
#include <vector>
#include <stack>

#define MAX_FILENAME 30

class PanelBrowser :
	public Panel
{
public:
	PanelBrowser();
	~PanelBrowser();

	bool Init() override;
	void Draw();

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
	std::string path;

private:
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
	ResourceTexture* folderIcon;	
	ResourceTexture* fileIcon;
	ResourceTexture* fbxIcon;
	ResourceTexture* pngIcon;
	ResourceTexture* jpgIcon;
	ResourceTexture* tgaIcon;
	ResourceTexture* tifIcon;
	ResourceTexture* ddsIcon;
	ResourceTexture* m4tIcon;
	ResourceTexture* jsonIcon;
	ResourceTexture* sc3neIcon;
	ResourceTexture* animati0nIcon;
	ResourceTexture* st4tem4chineIcon;
};

#endif //__PanelBrowser_h__