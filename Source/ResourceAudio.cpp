#include "ResourceAudio.h"

#include "Application.h"
#include "ModuleFileSystem.h"

#include "JSON.h"
#include "imgui.h"

ResourceAudio::ResourceAudio(unsigned uid) : Resource(uid, TYPE::AUDIO)
{
}

ResourceAudio::ResourceAudio(const ResourceAudio& resource) : Resource(resource)
{
}


ResourceAudio::~ResourceAudio()
{
	DeleteFromMemory();
}

bool ResourceAudio::LoadInMemory()
{
	if (Resource::IsLoadedToMemory())
		return false;

	bool success = false;

	int ret;

	if (!streamed) ret = wavFX.load(exportedFile.c_str());
	else ret = wavstream.load(exportedFile.c_str());

	//SO_NO_ERROR = 0, // No error
	//INVALID_PARAMETER = 1, // Some parameter is invalid
	//FILE_NOT_FOUND = 2,    // File not found
	//FILE_LOAD_FAILED = 3,  // File found, but could not be loaded

	if (ret != 0) 
	{
		LOG("ERROR: Audio Manager code: %i", ret);		
	}
	else 
	{
		LOG("Audio Manager: FX %s loaded \n", name.c_str());

		success = true;
	}

	// Increase references
	if (success) ++loaded;
	return success;
}

void ResourceAudio::DeleteFromMemory()
{
	Resource::DeleteFromMemory();
}

void ResourceAudio::SaveMetafile(const char * file) const
{
	std::string filepath;
	filepath.append(file);
	JSON* json = new JSON();
	JSON_value* meta = json->CreateValue();
	struct stat statFile;
	stat(filepath.c_str(), &statFile);
	meta->AddUint("GUID", UID);
	meta->AddUint("timeCreated", statFile.st_ctime);
	meta->AddUint("streamed", streamed);
	json->AddValue("Audio", *meta);
	filepath += METAEXT;
	App->fsystem->Save(filepath.c_str(), json->ToString().c_str(), json->Size());
	RELEASE(json);
}

void ResourceAudio::LoadConfigFromMeta()
{
	std::string metaFile(file);
	metaFile += ".meta";

	// Check if meta file exists
	if (!App->fsystem->Exists(metaFile.c_str()))
		return;

	char* data = nullptr;
	unsigned oldUID = GetUID();

	if (App->fsystem->Load(metaFile.c_str(), &data) == 0)
	{
		LOG("Warning: %s couldn't be loaded", metaFile.c_str());
		RELEASE_ARRAY(data);
		return;
	}
	JSON* json = new JSON(data);
	JSON_value* value = json->GetValue("Audio");

	streamed = value->GetUint("streamed");

	// Make sure the UID from meta is the same
	unsigned checkUID = value->GetUint("GUID");

	RELEASE(json);
}

void ResourceAudio::DrawImportConfiguration()
{
	ImGui::Checkbox("Streamed", &streamed);
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text("Streamed audio files will load into memory gradualy\n- Uncheck if its an FX");
		ImGui::EndTooltip();
	}
}