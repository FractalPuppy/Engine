#include "PanelBrowser.h"
#include "Globals.h"
#include "SDL.h"
#include "IL/il.h"
#include "assimp/version.h"
#include "GL/glew.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "rapidjson/rapidjson.h"
#include "physfs.h"

#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleResourceManager.h"
#include "ModuleFileSystem.h"
#include "ModuleInput.h"
#include "ModuleScene.h"
#include "GameObject.h"

#include "ResourceTexture.h"
#include "ResourcePrefab.h"

// Icons
#define FOLDER_ICON			IMPORTED_RESOURCES "folderIconBlue.dds"
#define FILE_ICON			IMPORTED_RESOURCES "fileIconBlue.dds"
#define FBX_ICON			IMPORTED_RESOURCES "fbxIconBlue.dds"
#define PNG_ICON			IMPORTED_RESOURCES "pngIconBlue.dds"
#define JPG_ICON			IMPORTED_RESOURCES "jpgIconBlue.dds"
#define TGA_ICON			IMPORTED_RESOURCES "tgaIconBlue.dds"
#define TIF_ICON			IMPORTED_RESOURCES "tifIconBlue.dds"
#define DDS_ICON			IMPORTED_RESOURCES "ddsIconBlue.dds"
#define M4T_ICON			IMPORTED_RESOURCES "m4tIconBlue.dds"
#define JSON_ICON			IMPORTED_RESOURCES "jsonIconBlue.dds"
#define SC3NE_ICON			IMPORTED_RESOURCES "sc3neIconBlue.dds"
#define ANIMATI0N_ICON		IMPORTED_RESOURCES "animati0nIconBlue.dds"
#define ST4TEM4CHINE_ICON	IMPORTED_RESOURCES "st4tem4chineIconBlue.dds"
#define WAV_ICON			IMPORTED_RESOURCES "wavIconBlue.dds"
#define OGG_ICON			IMPORTED_RESOURCES "oggIconBlue.dds"
#define MP3_ICON			IMPORTED_RESOURCES "mp3IconBlue.dds"
#define PR3FAB_ICON			IMPORTED_RESOURCES "pr3fabIconBlue.dds"

// ImGui elements sizes:
#define WINDOW_LOW_MARGIN 60
#define ICON_SIZE 40
#define ICON_X_MARGIN 20
#define ICON_Y_MARGIN 32
#define LEFT_INDENTATION 15
#define ICON_NAME_SIZE ICON_SIZE + 12
#define TEXT_SIZE 15

// |<--X Indentation--><--Icon Size--><--Icon X Margin--><--Next Icon---> _
// |				   _______________					 _______________  |
// |				  |				  |					|				| |
// |				  |				  |					|				| |
// |				  |				  |					|				| |
// |				  |				  |					|				| Icon Size
// |				  |				  |					|				| |
// |				  |				  |					|				| |
// |				  |_______________|					|_______________| _
// |					  IconName							IconName	  |
// |				  <--IconNameSize->					<--IconNameSize-> Icon Y Margin
// |																	  | 
// |				   _______________					 _______________  _
// |				  |				  |					|				| |
// |				  |				  |					|				| |
// |				  |				  |					|				| |
// |				  |				  |					|				| Icon Size
// |				  |				  |					|				| |
// |				  |				  |					|				| |
// |				  |_______________|					|_______________| _
// |					  IconName							IconName	  |
// |																	  |
// |																	  Window Low Margin
// |																	  |
// |______________________________________________________________________|

PanelBrowser::PanelBrowser() : path(ASSETS)
{
}

PanelBrowser::~PanelBrowser() 
{
	while (!pathStack.empty())
		pathStack.pop();

	files.clear();
	dirs.clear();

	fileSelected = nullptr;
}

bool PanelBrowser::Init()
{
	folderIcon = (ResourceTexture*)App->resManager->Get(FOLDER_ICON);
	fileIcon = (ResourceTexture*)App->resManager->Get(FILE_ICON);
	fbxIcon = (ResourceTexture*)App->resManager->Get(FBX_ICON);
	pngIcon = (ResourceTexture*)App->resManager->Get(PNG_ICON);
	jpgIcon = (ResourceTexture*)App->resManager->Get(JPG_ICON);
	tgaIcon = (ResourceTexture*)App->resManager->Get(TGA_ICON);
	tifIcon = (ResourceTexture*)App->resManager->Get(TIF_ICON);
	ddsIcon = (ResourceTexture*)App->resManager->Get(DDS_ICON);
	m4tIcon = (ResourceTexture*)App->resManager->Get(M4T_ICON);
	jsonIcon = (ResourceTexture*)App->resManager->Get(JSON_ICON);
	sc3neIcon = (ResourceTexture*)App->resManager->Get(SC3NE_ICON);
	animati0nIcon = (ResourceTexture*)App->resManager->Get(ANIMATI0N_ICON);
	st4tem4chineIcon = (ResourceTexture*)App->resManager->Get(ST4TEM4CHINE_ICON);
	wavIcon = (ResourceTexture*)App->resManager->Get(WAV_ICON);
	oggIcon = (ResourceTexture*)App->resManager->Get(OGG_ICON);
	mp3Icon = (ResourceTexture*)App->resManager->Get(MP3_ICON);
	pr3fabIcon = (ResourceTexture*)App->resManager->Get(PR3FAB_ICON);

	if (folderIcon == nullptr || fileIcon == nullptr || fbxIcon == nullptr || pngIcon == nullptr || jpgIcon == nullptr
		|| tgaIcon == nullptr || tifIcon == nullptr || ddsIcon == nullptr || m4tIcon == nullptr || jsonIcon == nullptr 
		|| sc3neIcon == nullptr || animati0nIcon == nullptr || st4tem4chineIcon == nullptr || wavIcon == nullptr 
		|| oggIcon == nullptr || mp3Icon == nullptr || pr3fabIcon == nullptr)
	{
		LOG("Warning: Some icons missing");
		return false;
	}

	return true;
}

void PanelBrowser::Draw() 
{
	if (!ImGui::Begin("Assets", &enabled))
	{
		ImGui::End();
		return;
	}

	// Get list of all files and directories when changes are detected
	if (folderContentDirty)
	{
		files.clear();
		dirs.clear();
		App->fsystem->ListFolderContent(path.c_str(), files, dirs);
		folderContentDirty = false;
	}

	// Draw current path
	ImGui::Text(path.c_str());

	// Icons area
	ImGui::BeginChild("Files", ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetWindowContentRegionMax().y - WINDOW_LOW_MARGIN), true, ImGuiWindowFlags_HorizontalScrollbar);

	int i = 0;
	// If we are inside a folder show icon to go back
	if (path != ASSETS)
	{
		ImGui::SetCursorPosY(ICON_Y_MARGIN);
		if (ImGui::ImageButton(folderIcon != nullptr ? (ImTextureID)folderIcon->gpuID : 0, ImVec2(ICON_SIZE, ICON_SIZE), ImVec2(0, 1), ImVec2(1, 0), 1))
		{
			path = pathStack.top();
			pathStack.pop();
			folderContentDirty = true;
		}

		ImGui::SetCursorPosY(ICON_SIZE + ICON_Y_MARGIN);	
		ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ICON_NAME_SIZE);
		ImGui::Text("..");
		if (ImGui::IsItemHovered() && App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN)
		{
			path = pathStack.top();
			pathStack.pop();
			folderContentDirty = true;
		}
		i++;
	}

	// Draw folder icons
	for each(std::string dir in dirs)
	{
		ImGui::PushID(dir.c_str());
		DrawFolderIcon(dir.c_str(), i);		
		i++;
		ImGui::PopID();
	}
	
	// Draw files icons
	for each (std::string file in files)
	{
		// Check that the file has been loaded on the Resource Manager
		if (App->resManager->FindByFileInAssets((path + file).c_str()) > 0)
		{
			DrawFileIcon(file.c_str(), i);
			i++;
		}
	}

	// RightClick on window without selecting an item
	if (App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_DOWN && ImGui::IsWindowFocused(ImGuiFocusedFlags_None))
	{
		ImGui::OpenPopup("Browser Context Menu");
	}

	// Right click menu
	DrawBrowserContextMenu();

	ImGui::EndChild();

	// Import Configuration Pop-up
	if (openImportConfigPopUp)
		DrawImportConfigurationPopUp();

	// Rename File Pop-up
	if (openRenameFilePopUp)
		DrawRenameFilePopUp();

	// Rename Folder Pop-up
	if (openRenameFolderPopUp)
		DrawRenameFolderPopUp();

	// New Folder Pop-up
	if (openNewFolderPopUp)
		DrawNewFolderPopUp();

	ImGui::End();	
}

void PanelBrowser::DrawFolderIcon(const char* dir, int itemNumber)
{
	ImGuiContext* context = ImGui::GetCurrentContext();
	ImVec2 size = context->CurrentWindow->Size;

	// Calculate number of items per row
	int maxNumberElements = size.x / (ICON_SIZE + ICON_X_MARGIN);
	if (maxNumberElements < 1) maxNumberElements = 1;

	// Folder icon position
	ImGui::SetCursorPosX(LEFT_INDENTATION + (ICON_SIZE + ICON_X_MARGIN) * (itemNumber % maxNumberElements));
	ImGui::SetCursorPosY(ICON_Y_MARGIN + (ICON_SIZE + ICON_Y_MARGIN) * (itemNumber / maxNumberElements));

	if (ImGui::ImageButton(folderIcon != nullptr ? (ImTextureID)folderIcon->gpuID : 0, ImVec2(ICON_SIZE, ICON_SIZE), ImVec2(0, 1), ImVec2(1, 0), 1))
	{
		pathStack.push(path);
		path += dir;
		path += "/";
		folderContentDirty = true;
	}

	if (ImGui::IsItemHovered() && App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_DOWN)
	{
		folderSelected = dir;
		ImGui::OpenPopup("Folder Context Menu");
	}

	// Folder name text
	ImGui::SetCursorPosX(LEFT_INDENTATION + (ICON_SIZE + ICON_X_MARGIN) * (itemNumber % maxNumberElements));
	ImGui::SetCursorPosY((ICON_SIZE + ICON_Y_MARGIN) + (ICON_SIZE + ICON_Y_MARGIN) * (itemNumber / maxNumberElements));
	ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ICON_NAME_SIZE);

	// Make sure the text fits the designated size.
	std::string dirText(dir, TEXT_SIZE);
	if (strlen(dirText.c_str()) >= TEXT_SIZE)
	{
		unsigned size2 = strlen(dirText.c_str());
		// If not delete last 3 characters and put "..."
		dirText.replace(TEXT_SIZE - 4, TEXT_SIZE - 1, "...");
	}

	ImGui::Text(dirText.c_str());
	if (ImGui::IsItemHovered())
	{
		if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN)
		{
			pathStack.push(path);
			path += dir;
			path += "/";
			folderContentDirty = true;
		}
		else if (App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_DOWN)
		{
			folderSelected = dir;
			ImGui::OpenPopup("Folder Context Menu");
		}
	}
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) && !ImGui::IsPopupOpen("Folder Context Menu"))
	{
		ImGui::BeginTooltip();
		ImGui::Text(dir);
		ImGui::EndTooltip();
	}


	// Right click menu
	DrawFolderContextMenu();
}

void PanelBrowser::DrawFileIcon(const char* file, int itemNumber)
{
	ImGui::PushID(file);
	ImGuiContext* context = ImGui::GetCurrentContext();
	ImVec2 size = context->CurrentWindow->Size;

	// Calculate number of items per row
	int maxNumberElements = size.x / (ICON_SIZE + ICON_X_MARGIN);
	if (maxNumberElements < 1) maxNumberElements = 1;

	// File icon position
	ImGui::SetCursorPosX(LEFT_INDENTATION + (ICON_SIZE + ICON_X_MARGIN) * (itemNumber % maxNumberElements));
	ImGui::SetCursorPosY(ICON_Y_MARGIN + (ICON_SIZE + ICON_Y_MARGIN) * (itemNumber / maxNumberElements));

	// Select icon by extension
	std::string extension = App->fsystem->GetExtension(file);
	if		(extension == PNG)					{ ImGui::ImageButton(pngIcon != nullptr ? (ImTextureID)pngIcon->gpuID : 0, ImVec2(40, 40), ImVec2(0, 1), ImVec2(1, 0), 1); }
	else if (extension == TIF)					{ ImGui::ImageButton(tifIcon != nullptr ? (ImTextureID)tifIcon->gpuID : 0, ImVec2(40, 40), ImVec2(0, 1), ImVec2(1, 0), 1); }
	else if (extension == JPG)					{ ImGui::ImageButton(jpgIcon != nullptr ? (ImTextureID)jpgIcon->gpuID : 0, ImVec2(40, 40), ImVec2(0, 1), ImVec2(1, 0), 1); }
	else if (extension == TGA)					{ ImGui::ImageButton(tgaIcon != nullptr ? (ImTextureID)tgaIcon->gpuID : 0, ImVec2(ICON_SIZE, ICON_SIZE), ImVec2(0, 1), ImVec2(1, 0), 1); }
	else if (extension == TEXTUREEXT)			{ ImGui::ImageButton(ddsIcon != nullptr ? (ImTextureID)ddsIcon->gpuID : 0, ImVec2(ICON_SIZE, ICON_SIZE), ImVec2(0, 1), ImVec2(1, 0), 1); }
	else if (extension == FBXEXTENSION)			{ ImGui::ImageButton(fbxIcon != nullptr ? (ImTextureID)fbxIcon->gpuID : 0, ImVec2(ICON_SIZE, ICON_SIZE), ImVec2(0, 1), ImVec2(1, 0), 1); }
	else if (extension == MATERIALEXT)			{ ImGui::ImageButton(m4tIcon != nullptr ? (ImTextureID)m4tIcon->gpuID : 0, ImVec2(ICON_SIZE, ICON_SIZE), ImVec2(0, 1), ImVec2(1, 0), 1); }
	else if (extension == SCENEEXTENSION)		{ ImGui::ImageButton(sc3neIcon != nullptr ? (ImTextureID)sc3neIcon->gpuID : 0, ImVec2(ICON_SIZE, ICON_SIZE), ImVec2(0, 1), ImVec2(1, 0), 1); }
	else if (extension == ANIMATIONEXTENSION)	{ ImGui::ImageButton(animati0nIcon != nullptr ? (ImTextureID)animati0nIcon->gpuID : 0, ImVec2(ICON_SIZE, ICON_SIZE), ImVec2(0, 1), ImVec2(1, 0), 1); }
	else if (extension == STATEMACHINEEXTENSION){ ImGui::ImageButton(st4tem4chineIcon != nullptr ? (ImTextureID)st4tem4chineIcon->gpuID : 0, ImVec2(ICON_SIZE, ICON_SIZE), ImVec2(0, 1), ImVec2(1, 0), 1); }
	else if (extension == WAVEXTENSION)			{ ImGui::ImageButton(wavIcon != nullptr ? (ImTextureID)wavIcon->gpuID : 0, ImVec2(ICON_SIZE, ICON_SIZE), ImVec2(0, 1), ImVec2(1, 0), 1); }
	else if (extension == OGGEXTENSION)			{ ImGui::ImageButton(oggIcon != nullptr ? (ImTextureID)oggIcon->gpuID : 0, ImVec2(ICON_SIZE, ICON_SIZE), ImVec2(0, 1), ImVec2(1, 0), 1); }
	else if (extension == MP3EXTENSION)			{ ImGui::ImageButton(mp3Icon != nullptr ? (ImTextureID)mp3Icon->gpuID : 0, ImVec2(ICON_SIZE, ICON_SIZE), ImVec2(0, 1), ImVec2(1, 0), 1); }
	else if (extension == PREFABEXTENSION)		{ ImGui::ImageButton(pr3fabIcon != nullptr ? (ImTextureID)pr3fabIcon->gpuID : 0, ImVec2(ICON_SIZE, ICON_SIZE), ImVec2(0, 1), ImVec2(1, 0), 1); }
	else										{ ImGui::ImageButton(fileIcon != nullptr ? (ImTextureID)fileIcon->gpuID : 0, ImVec2(ICON_SIZE, ICON_SIZE), ImVec2(0, 1), ImVec2(1, 0), 1); }

	if (ImGui::IsItemHovered() && App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_DOWN)
	{
		TYPE resourceType = App->resManager->GetResourceType(App->fsystem->GetFileType(extension));
		unsigned selectedUID = App->resManager->FindByFileInAssetsOfType((path + file).c_str(), resourceType);
		fileSelected = App->resManager->GetWithoutLoad(selectedUID);
		ImGui::OpenPopup("File Context Menu");
	}
	else if (ImGui::IsItemHovered() && App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN)
	{
		if (App->fsystem->GetExtension(file) == PREFABEXTENSION)
		{
			unsigned selectedUID = App->resManager->FindByFileInAssetsOfType((path + file).c_str(), TYPE::PREFAB);

			dragAsset = new GameObject();
			dragAsset->UUID = 2; //dummy value for passing uuid > 1 check
			dragAsset->isPrefab = true;
			dragAsset->prefabUID = selectedUID;
			dragAsset->isDropablePlaceHolder = true;
			dragAsset->name = "Prefab";
		}
		//else TODO: Scenes or FBX drop
	}

	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT && dragAsset != nullptr)
	{
		App->scene->DragNDrop(dragAsset);
	}
	else if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_UP && dragAsset != nullptr)
	{
		dragAsset->CleanUp();
		RELEASE(dragAsset);
	}

	ImGui::SetCursorPosX(LEFT_INDENTATION + (ICON_SIZE + ICON_X_MARGIN) * (itemNumber % maxNumberElements));
	ImGui::SetCursorPosY((ICON_SIZE + ICON_Y_MARGIN) + (ICON_SIZE + ICON_Y_MARGIN) * (itemNumber / maxNumberElements));

	ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + 52);
	
	// Make sure the text fits the designated size.
	std::string fileText(file, TEXT_SIZE);
	if (strlen(fileText.c_str()) >= TEXT_SIZE)
	{
		// If not delete last 3 characters and put "..."
		fileText.replace(TEXT_SIZE - 4, TEXT_SIZE - 1, "...");
	}

	ImGui::Text(fileText.c_str());

	if (ImGui::IsItemHovered() && App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_DOWN)
	{
		TYPE resourceType = App->resManager->GetResourceType(App->fsystem->GetFileType(extension));
		unsigned selectedUID = App->resManager->FindByFileInAssetsOfType((path + file).c_str(), resourceType);
		fileSelected = App->resManager->GetWithoutLoad(selectedUID);
		ImGui::OpenPopup("File Context Menu");
	}
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) && !ImGui::IsPopupOpen("File Context Menu"))
	{
		ImGui::BeginTooltip();
		ImGui::Text(file);
		ImGui::EndTooltip();
	}

	// Right click menu
	DrawFileContextMenu();
	ImGui::PopID();
}

void PanelBrowser::DrawBrowserContextMenu()
{
	if (ImGui::BeginPopup("Browser Context Menu"))
	{
		if (ImGui::Selectable("New Folder"))
		{
			openNewFolderPopUp = true;
		}
		ImGui::EndPopup();
	}
}

void PanelBrowser::DrawFileContextMenu()
{
	if (ImGui::BeginPopup("File Context Menu"))
	{
		if (fileSelected == nullptr)
		{ 
			ImGui::EndPopup();
			return;
		}

		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), App->fsystem->GetFile(fileSelected->GetFile()).c_str());
		ImGui::Separator();

		if (ImGui::Selectable("Rename"))
		{
			strcpy(newFileName, App->fsystem->GetFilename(fileSelected->GetFile()).c_str());
			openRenameFilePopUp = true;
		}
		if(ImGui::Selectable("Delete"))
		{
			fileSelected->Delete();
			folderContentDirty = true;
		}
		if (ImGui::Selectable("Import Configuration"))
		{
			//Code to change import settings
			openImportConfigPopUp = true;
		}	
		ImGui::EndPopup();
	}
}

void PanelBrowser::DrawFolderContextMenu()
{
	if (ImGui::BeginPopup("Folder Context Menu"))
	{
		if (folderSelected.empty())
		{
			ImGui::EndPopup();
			return;
		}

		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), folderSelected.c_str());
		ImGui::Separator();

		if (ImGui::Selectable("Rename"))
		{
			strcpy(newFolderName, folderSelected.c_str());
			openRenameFolderPopUp = true;
		}
		if (ImGui::Selectable("Delete"))
		{
			App->fsystem->Delete((path + folderSelected).c_str());
			folderContentDirty = true;
		}
		ImGui::EndPopup();
	}
}

void PanelBrowser::DrawImportConfigurationPopUp()
{
	ImGui::OpenPopup("Import configuration");

	ImGui::SetNextWindowSizeConstraints(ImVec2(250.0f, 130.0f), ImVec2((float)App->window->width, (float)App->window->height));
	if (ImGui::BeginPopupModal("Import configuration", &openImportConfigPopUp))
	{
		ImGui::Text("%s", fileSelected->GetExportedFile());
		fileSelected->DrawImportConfiguration();

		if (ImGui::Button("Accept"))
		{
			// Add accept logic
			openImportConfigPopUp = false;
			App->resManager->ReImportFile(fileSelected, path.c_str(), fileSelected->GetType());
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			openImportConfigPopUp = false;
		}
		ImGui::EndPopup();
	}
}

void PanelBrowser::DrawRenameFilePopUp()
{
	ImGui::OpenPopup("Rename File");

	ImGui::SetNextWindowSizeConstraints(ImVec2(270.0f, 150.0f), ImVec2((float)App->window->width, (float)App->window->height));
	if (ImGui::BeginPopupModal("Rename File", &openRenameFilePopUp))
	{
		ImGui::Text("New name:");
		ImGui::SameLine();

		if (ImGui::InputText("", newFileName, MAX_FILENAME, ImGuiInputTextFlags_EnterReturnsTrue) && !invalidName)
		{
			fileSelected->Rename(newFileName);
			folderContentDirty = true;
			strcpy(newFileName, "");
			openRenameFilePopUp = false;
		}

		// Check if there isn't already a file with the same name
		invalidName = App->resManager->NameExists(newFileName, fileSelected->GetType());

		if(invalidName)
			ImGui::Text("A file with that name already exists!");
		else
			ImGui::NewLine();

		ImGui::NewLine();
		if (ImGui::ButtonEx("Accept", ImVec2(0, 0), invalidName? ImGuiButtonFlags_Disabled : 0))
		{
			fileSelected->Rename(newFileName);
			folderContentDirty = true;
			strcpy(newFileName,"");
			openRenameFilePopUp = false;
		}
		ImGui::SameLine(0, 110);
		if (ImGui::Button("Cancel"))
		{
			strcpy(newFileName, "");
			openRenameFilePopUp = false;
		}
		ImGui::EndPopup();
	}
}

void PanelBrowser::DrawRenameFolderPopUp()
{
	ImGui::OpenPopup("Rename Folder");

	ImGui::SetNextWindowSizeConstraints(ImVec2(270.0f, 150.0f), ImVec2((float)App->window->width, (float)App->window->height));
	if (ImGui::BeginPopupModal("Rename Folder", &openRenameFolderPopUp))
	{
		ImGui::Text("New name:");
		ImGui::SameLine();

		if (ImGui::InputText("", newFolderName, MAX_FILENAME, ImGuiInputTextFlags_EnterReturnsTrue) && !invalidName)
		{
			App->fsystem->Rename(path.c_str(), folderSelected.c_str(), newFolderName);
			folderContentDirty = true;
			strcpy(newFolderName, "");
			openRenameFolderPopUp = false;
		}

		// Check if there isn't already a file with the same name
		invalidName = App->fsystem->Exists((path + newFolderName).c_str());

		if (invalidName)
			ImGui::Text("A folder with that name already exists!");
		else
			ImGui::NewLine();

		ImGui::NewLine();
		if (ImGui::ButtonEx("Accept", ImVec2(0, 0), invalidName ? ImGuiButtonFlags_Disabled : 0))
		{
			App->fsystem->Rename(path.c_str(), folderSelected.c_str(), newFolderName);
			folderContentDirty = true;
			strcpy(newFolderName, "");
			openRenameFolderPopUp = false;
		}
		ImGui::SameLine(0, 110);
		if (ImGui::Button("Cancel"))
		{
			strcpy(newFolderName, "");
			openRenameFolderPopUp = false;
		}
		ImGui::EndPopup();
	}
}

void PanelBrowser::DrawNewFolderPopUp()
{
	ImGui::OpenPopup("New Folder");

	ImGui::SetNextWindowSizeConstraints(ImVec2(270.0f, 150.0f), ImVec2((float)App->window->width, (float)App->window->height));
	if (ImGui::BeginPopupModal("New Folder", &openNewFolderPopUp))
	{
		ImGui::Text("New folder name:");
		ImGui::SameLine();

		if (ImGui::InputText("", newFolderName, MAX_FILENAME, ImGuiInputTextFlags_EnterReturnsTrue) && !invalidName)
		{
			App->fsystem->MakeDirectory((path + newFolderName).c_str());
			folderContentDirty = true;
			strcpy(newFolderName, "");
			openNewFolderPopUp = false;
		}

		// Check if there isn't already a file with the same name
		invalidName = App->fsystem->Exists((path + newFolderName).c_str());

		if (invalidName)
			ImGui::Text("A folder with that name already exists!");
		else
			ImGui::NewLine();

		ImGui::NewLine();
		if (ImGui::ButtonEx("Accept", ImVec2(0, 0), invalidName ? ImGuiButtonFlags_Disabled : 0))
		{
			App->fsystem->MakeDirectory((path + newFolderName).c_str());
			folderContentDirty = true;
			strcpy(newFolderName, "");
			openNewFolderPopUp = false;
		}
		ImGui::SameLine(0, 110);
		if (ImGui::Button("Cancel"))
		{
			strcpy(newFolderName, "");
			openNewFolderPopUp = false;
		}
		ImGui::EndPopup();
	}
}