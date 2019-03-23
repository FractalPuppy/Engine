#include "Resource.h"
#include "JSON.h"

#include "Application.h"
#include "ModuleResourceManager.h"

Resource::Resource(unsigned uid, TYPE type): UID(uid), type(type)
{
}

Resource::Resource(const Resource& resource)
{
	UID = App->resManager->GenerateNewUID();
	file = resource.file;
	exportedFileName = resource.exportedFileName;
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
	exportedFileName = resource.exportedFileName;
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

void Resource::Load(const JSON_value &config)
{
	UID = config.GetUint("UID");
	file = config.GetString("File");
	exportedFileName = config.GetString("ExportedFile");

	type = (TYPE)config.GetUint("Type");
}