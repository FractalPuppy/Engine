#include "FileExplorer.h"

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleScene.h"

#include "imgui.h"

FileExplorer::FileExplorer()
{
	extensionToFilter = FILETYPE::NONE;
}

FileExplorer::~FileExplorer()
{
	Reset();
}

void FileExplorer::Reset()
{
	sprintf_s(title, "File Explorer");
	sprintf_s(filename, "");
	extensionToFilter = FILETYPE::NONE;
	currentOperation = MenuOperations::NONE;
	ResetPath();
}

void FileExplorer::ResetPath()
{
	path = "Assets";
	while (!pathStack.empty())
		pathStack.pop();
}

bool FileExplorer::Open()
{
	if (ImGui::BeginPopupModal(title, &openFileExplorer))
	{
		DrawPath();
		// List of files and directories box: [..] [Directories] [Files]
		std::vector<std::string> files;
		std::vector<std::string> dirs;
		ImGui::BeginChild("Files", ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetWindowContentRegionMax().y - 135), true, ImGuiWindowFlags_HorizontalScrollbar);
		App->fsystem->ListFolderContent(path.c_str(), files, dirs);

		ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(.0f, 0.5f));
		// [..] (Goes back one dir)
		if (pathStack.size() > 0)
		{
			if (ImGui::Selectable("..", false))
			{
				path = pathStack.top();
				pathStack.pop();
			}
		}
		// [Directories]
		for each (std::string dir in dirs)
		{
			std::string slash_dir = dir + "/";
			if (ImGui::Selectable(slash_dir.c_str(), false))
			{
				pathStack.push(path);
				path = path + "/" + dir;
			}
		}
		// [Files]
		for each (std::string file in files)
		{
			FilterByFileType(file.c_str());
		}
		ImGui::PopStyleVar();
		ImGui::EndChild();

		// File bar: [Text (File name:)] [Text input for filename] [Combo selector to filter extensions] 
		ImGui::Text("File Name:");
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth() * 0.55f);
		ImGui::InputText("", &filename[0], sizeof(filename));
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth() * 0.3f);
		char* program_names[(int)FILETYPE::NONE+1] = { "Texture Files (*.png, *.tif, *.jpg)", "Imported Texture Files (*.dds)", "Model Files (*.fbx, *.FBX)", "Imported Mesh Files (*.m3sh)", "Scene Files (*.sc3ne)", "Material (*.m4t)", "All files" };
		ImGui::Combo(" ", (int*)&extensionToFilter, program_names, (int)FILETYPE::NONE + 1);
		ImGui::PopItemWidth();

		// Accept button (changes text depending on operation)
		switch (currentOperation)
		{
		case MenuOperations::SAVE:
			ImGui::Checkbox("Save selected game objects only", &saveSelected); 
			ImGui::SameLine();
			if (ImGui::Button("Save", ImVec2(100, 20)))
			{
				openFileExplorer = false;
				ImGui::EndPopup();
				return true;
			}
			break;
		case MenuOperations::ADD:
			if (ImGui::Button("Add", ImVec2(100, 20)))
			{
				openFileExplorer = false;
				ImGui::EndPopup();
				return true;
			}
			break;
		case MenuOperations::LOAD:
			if (ImGui::Button("Load", ImVec2(100, 20)))
			{
				openFileExplorer = false;
				ImGui::EndPopup();
				return true;
			}
			break;
		default:
		case MenuOperations::NONE:
			if (ImGui::Button("Accept", ImVec2(100, 20)))
			{
				openFileExplorer = false;
				ImGui::EndPopup();
				return true;
			}
			break;
		}
		ImGui::SameLine();

		// Cancel button
		if (ImGui::Button("Cancel", ImVec2(100, 20)))
		{
			openFileExplorer = false;
			Reset();
			ImGui::EndPopup();
			return false;
		}		
		ImGui::EndPopup();
	}
	return false;
}

void FileExplorer::Draw()
{
	if (openFileExplorer)
	{
		ImGui::SetNextWindowSizeConstraints(ImVec2(600, 300), ImVec2(1200, 800));
		ImGui::OpenPopup(title);

		if (Open())
		{
			switch (currentOperation)
			{
			case MenuOperations::SAVE:
				App->scene->SaveScene(*App->scene->root, filename, (path + "/").c_str(), saveSelected);
				break;
			case MenuOperations::LOAD:
				App->scene->LoadScene(filename, (path + "/").c_str());
				break;
			case MenuOperations::ADD:
				App->scene->AddScene(filename, (path + "/").c_str());
			}
			Reset();
		}
	}
}

void FileExplorer::DrawPath()
{
	std::vector<std::string> folders = GetPath();
	if (ImGui::Button("Assets"))
	{
		ResetPath();
	}
	for each (std::string folder in folders)
	{
		ImGui::SameLine();
		if (ImGui::Button(folder.c_str()))
		{
			if (folder != folders[folders.size() - 1])
			{
				while (pathStack.top().substr(pathStack.top().find_last_of('/')) != folder)
				{
					pathStack.pop();
				}
				if (pathStack.top().substr(pathStack.top().find_last_of('/')) == folder)
				{
					path = pathStack.top();
					pathStack.pop();
				}
			}
		}
	}
}

void FileExplorer::FilterByFileType(const char* file)
{
	std::string extension = "";
	switch (extensionToFilter)
	{
	case FILETYPE::TEXTURE:
		extension = App->fsystem->GetExtension(file);
		if (extension == PNG || extension == TIF || extension == JPG)
		{
			if (ImGui::Selectable(file, false))
				sprintf_s(filename, App->fsystem->GetFilename(file).c_str());
		}
		break;
	case FILETYPE::IMPORTED_TEXTURE:
		extension = App->fsystem->GetExtension(file);
		if (extension == TEXTUREEXT)
		{
			if (ImGui::Selectable(file, false))
				sprintf_s(filename, App->fsystem->GetFilename(file).c_str());
		}
		break;
	case FILETYPE::MODEL:
		extension = App->fsystem->GetExtension(file);
		if (extension == FBXEXTENSION || extension == FBXCAPITAL)
		{
			if (ImGui::Selectable(file, false))
				sprintf_s(filename, App->fsystem->GetFilename(file).c_str());
		}
		break;
	case FILETYPE::IMPORTED_MESH:
		extension = App->fsystem->GetExtension(file);
		if (extension == MESHEXTENSION)
		{
			if (ImGui::Selectable(file, false))
				sprintf_s(filename, App->fsystem->GetFilename(file).c_str());
		}
		break;
	case FILETYPE::SCENE:
		extension = App->fsystem->GetExtension(file);
		if (extension == SCENEEXTENSION)
		{
			if (ImGui::Selectable(file, false))
				sprintf_s(filename, App->fsystem->GetFilename(file).c_str());
		}
		break;
	case FILETYPE::MATERIAL:
		extension = App->fsystem->GetExtension(file);
		if (extension == MATERIALEXT)
		{
			if (ImGui::Selectable(file, false))
				sprintf_s(filename, App->fsystem->GetFilename(file).c_str());
		}
		break;
	case FILETYPE::AUDIO:
		extension = App->fsystem->GetExtension(file);
		if (extension == WAVEXTENSION || extension == OGGEXTENSION || extension == MP3EXTENSION)
		{
			if (ImGui::Selectable(file, false))
				sprintf_s(filename, App->fsystem->GetFilename(file).c_str());
		}
		break;
	default:
	case FILETYPE::NONE:
		if (ImGui::Selectable(file, false))
			sprintf_s(filename, App->fsystem->GetFilename(file).c_str());
		break;
	}
}

std::vector<std::string> FileExplorer::GetPath()
{
	std::vector<int> separatorLocations;
	std::vector<std::string> auxFolders;
	for (int i = 0; i < path.size(); i++)
		if (path[i] == '/')
			separatorLocations.push_back(i);
	for (std::vector<int>::iterator it = separatorLocations.begin(); it != separatorLocations.end(); ++it)
	{
		if (it != separatorLocations.end() - 1)
		{
			auxFolders.push_back(path.substr((*it), (*(it + 1)) - (*it)));
		}
		else
		{
			auxFolders.push_back(path.substr((*it), (path.size() - (*it))));
		}
	}
	return auxFolders;
}

std::vector<std::string> FileExplorer::GetPath(std::string prevPath)
{
	std::vector<int> separatorLocations;
	std::vector<std::string> auxFolders;
	for (int i = 0; i < prevPath.size(); i++)
		if (prevPath[i] == '/')
			separatorLocations.push_back(i);
	for (std::vector<int>::iterator it = separatorLocations.begin(); it != (separatorLocations.end()); ++it)
	{
		if (it == separatorLocations.begin())
		{
			auxFolders.push_back(prevPath.substr(0, *it));
		}
		else
		{
			auxFolders.push_back(prevPath.substr(*(it-1), *it-*(it-1)));
		}
	}
	return auxFolders;
}

void FileExplorer::SetPath(const char* newPath)
{
	std::string scenePath(newPath);
	std::vector<std::string> prevPath = GetPath(scenePath.substr(0, scenePath.size() - 1));

	path = scenePath;
	for (int i = 0; i < prevPath.size(); ++i)
	{
		pathStack.push(prevPath[i]);
	}
}

void FileExplorer::OpenFileExplorer(MenuOperations operation, FILETYPE typeToFilter, const char* startPath, const char* windowTitle, const char* fileName) 
{
	currentOperation = operation;
	extensionToFilter = typeToFilter;
	SetPath(startPath);
	sprintf_s(title, windowTitle);
	sprintf_s(filename, fileName);
	openFileExplorer = true;
	saveSelected = false;
}