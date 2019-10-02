#include "PlayerPrefs.h"
#include "Application.h"
#include "ModuleFileSystem.h"
#include "JSON.h"

#define PREFS ".prefs"
#define PREFSFILE PERSISTENCE "player"

std::map<const char*, int, ltstr> PlayerPrefs::integers;
std::map<const char*, float, ltstr> PlayerPrefs::floats;
std::map<const char*, const char*, ltstr> PlayerPrefs::strings;
bool PlayerPrefs::hasPrefsFile;

void PlayerPrefs::Save()
{
	if (integers.empty() && floats.empty() && strings.empty() && hasPrefsFile)
	{
		App->fsystem->Delete(PREFSFILE PREFS);
		hasPrefsFile = false;
		return;
	}
	unsigned totalSize = GetMapSize(integers) + GetMapSize(floats) + GetMapSize(strings);
	char *buffer = new char[totalSize];
	char* cursor = buffer;

	//copying....
	SerializeMap(integers, &cursor);
	SerializeMap(floats, &cursor);
	SerializeMap(strings, &cursor);
	App->fsystem->Save(PREFSFILE PREFS, buffer, totalSize);
	RELEASE_ARRAY(buffer);
}

void PlayerPrefs::Load()
{
	char* data = nullptr;
	unsigned size = App->fsystem->Load(PREFSFILE PREFS, &data);
	if (size > 0)
	{
		hasPrefsFile = true;
		char* cursor = data;
		DeSerializeMap(integers, &cursor);
		DeSerializeMap(floats, &cursor);
		DeSerializeMap(strings, &cursor);
	}

	RELEASE_ARRAY(data);
}


int PlayerPrefs::GetInt(const char* key, int defaultValue)
{
	return Get(integers, key, defaultValue);
}

float PlayerPrefs::GetFloat(const char* key, float defaultValue)
{
	return Get(floats, key, defaultValue);
}

const char * PlayerPrefs::GetString(const char* key, const char* defaultValue)
{
	return Get(strings, key, defaultValue);
}

void PlayerPrefs::SetInt(const char* key, int value)
{
	Set(integers, key, value);
}

void PlayerPrefs::SetFloat(const char* key, float value)
{
	Set(floats, key, value);
}

void PlayerPrefs::SetString(const char* key, const char* value)
{
	assert(value != nullptr);
	char* cpy = new char[strlen(value) + 1];
	strcpy(cpy, value);

	Set(strings, key, (const char*)cpy);
}

bool PlayerPrefs::HasKey(const char* key)
{
	return Find(integers, key) || Find(floats, key) || Find(strings, key);
}

void PlayerPrefs::DeleteKey(const char* key)
{
	if (Delete(integers, key)) return;
	if (Delete(floats, key)) return;
	if (Delete(strings, key)) return;
}

void PlayerPrefs::DeleteAll(bool deleteJsons)
{
	CleanMap(integers);
	CleanMap(floats);
	CleanMap(strings);
	if (deleteJsons)
	{
		std::set<std::string> files;
		App->fsystem->ListFileNames(PERSISTENCE, files);
		for (const auto& file : files)
		{
			DeleteJson(file.c_str());
		}
	}
}

JSON* PlayerPrefs::LoadJson(const char* file)
{
	char* data = nullptr;
	std::string filepath(PERSISTENCE);
	filepath += file;
	filepath += PREFS;

	App->fsystem->Load(filepath.c_str(), &data);
	JSON* json = nullptr;
	if (data != nullptr)
	{
		return new JSON(data);
	}
	return nullptr;
}

bool PlayerPrefs::SaveJson(JSON* json, const char* filename)
{
	assert(strcmp(filename, "player") != 0); //Cannot have this filename
	std::string filepath(PERSISTENCE);
	filepath += filename;
	filepath += PREFS;
	return App->fsystem->Save(filepath.c_str(), json->ToString().c_str(), json->Size());
}

void PlayerPrefs::DeleteJson(const char* file)
{
	std::string filepath(PERSISTENCE);
	filepath += file;
	filepath += PREFS;
	App->fsystem->Delete(filepath.c_str());
}
