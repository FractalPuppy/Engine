#include "Resource.h"
#include "JSON.h"

#include "Application.h"
#include "ModuleResourceManager.h"
#include "ModuleFileSystem.h"

#include "imgui.h"

Resource::Resource(unsigned uid, TYPE type): UID(uid), type(type)
{
}

Resource::Resource(const Resource& resource)
{
	UID = App->resManager->GenerateNewUID();
	name = resource.name;
	file = resource.file;
	exportedFile = resource.exportedFile;
	loaded = resource.loaded;
	engineUsed = resource.engineUsed;
	type = resource.type;
}

Resource::~Resource()
{
	DeleteFromMemory();
}

void Resource::Copy(const Resource& resource)
{
	file = resource.file;
	name = resource.name;
	exportedFile = resource.exportedFile;
	SetReferences(resource.loaded);
	engineUsed = resource.engineUsed;
	type = resource.type;
}

void Resource::SetReferences(unsigned references)
{
	if (references < 1 && IsLoadedToMemory())		// Delete from memory on loaded changed from > 0 to 0 
		DeleteFromMemory();
	else if (references > 0 && !IsLoadedToMemory())	// Load to memory on increase of loaded from 0 to > 0
		LoadInMemory();

	loaded = references;
}

void Resource::SaveMetafile(const char * file) const
{
}

bool Resource::ReloadInMemory()
{
	if (!IsLoadedToMemory())
	{
		return true;
	}
	else
	{
		unsigned references = loaded;
		DeleteFromMemory();
		if (LoadInMemory())
		{
			SetReferences(references);
			return true;
		}
	}
	return false;
}

void Resource::Rename(const char* newName)
{
	std::string ruteToFile = App->fsystem->GetFilePath(file);
	std::string fileInAssets = App->fsystem->GetFile(file);
	std::string extension = App->fsystem->GetExtension(file);

	//TODO RM: Add CheckNameAvailability
	name = newName;

	// Rename file in Assets
	if (type != TYPE::MESH && !fileInAssets.empty())
	{ 
		App->fsystem->Rename(ruteToFile.c_str(), fileInAssets.c_str(), newName);

		// Rename meta file in Assets
		std::string newMeta(newName);
		newMeta += extension;
		App->fsystem->Rename(ruteToFile.c_str(), (fileInAssets + ".meta").c_str(), newMeta.c_str());

		// Update file variable
		file = ruteToFile + newName + extension;
	}

	// Update meta info
	SaveMetafile(file.c_str());
}

void Resource::Delete()
{
	// Delete Resource from ResourceManager
	App->resManager->DeleteResourceFromList(UID);

	// Delete file in Assets
	App->fsystem->Delete(file.c_str());

	// Delete meta file in Assets
	App->fsystem->Delete((file + ".meta").c_str());

	// Delete file in Library
	App->fsystem->Delete(exportedFile.c_str());
	DeleteFromMemory();
}

void Resource::DrawImportConfiguration()
{
	ImGui::Text("No import options.");
}

void Resource::DrawLoadSettings()
{
	ImGui::Text("No load settings.");
}
