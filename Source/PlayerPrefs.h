#ifndef __PLAYERPREFS_H__
#define __PLAYERPREFS_H__

#include <map>
#include <assert.h>
#include "Globals.h"

class JSON;

struct ltstr
{
	bool operator()(const char* s1, const char* s2) const
	{
		return strcmp(s1, s2) < 0;
	}
};

class ENGINE_API PlayerPrefs
{
public:

	static void			Save();
	static void			Load();
	static int			GetInt(const char* key, int defaultValue = 0);
	static float		GetFloat(const char* key, float defaultValue = 0.0f);
	static const char*	GetString(const char* key, const char* defaultValue = nullptr);
	static void			SetInt(const char* key, int value);
	static void			SetFloat(const char* key, float value);
	static void			SetString(const char* key, const char* value);

	static bool			HasKey(const char* key);

	static void			DeleteKey(const char* key);
	static void			DeleteAll(bool deleteJsons = false); //flag also deletes all json files saved

	static JSON*		LoadJson(const char* file);
	static bool			SaveJson(JSON* json, const char* filename);
	static void			DeleteJson(const char* file);

private:

	template <class T>
	static unsigned GetMapSize(const std::map<const char*, T, ltstr>& myMap);

	template <class T>
	static void SerializeMap(const std::map<const char*, T, ltstr>& myMap, char** cursor);
	
	template <class T>
	static void DeSerializeMap(std::map<const char*, T, ltstr>& myMap, char** cursor);

	template <class T>
	static void DeSerializeValue(T& value, char** cursor);
	
	template <class T>
	static T Get(const std::map<const char*, T, ltstr>& myMap, const char* key, T defaultValue);
	
	template <class T>
	static void Set(std::map<const char*, T, ltstr>& myMap, const char* key, T value);
	
	template <class T>
	static void CleanMap(std::map<const char*, T, ltstr>& myMap);
	
	template <class T>
	static void ClearPosition(const char* key, T& value);
	
	template <class T>
	static bool Find(const std::map<const char*, T, ltstr>& myMap, const char* key);

	template <class T>
	static typename std::map<const char*, T, ltstr>::const_iterator FindIter(const std::map<const char*, T, ltstr>& myMap, const char* key);

	template <class T>
	static bool Delete(std::map<const char*, T, ltstr>& myMap, const char* key);

	template <class T>
	static void SaveValue(char** cursor, T value);

private:
	static std::map<const char*, int, ltstr> integers;
	static std::map<const char*, float, ltstr> floats;
	static std::map<const char*, const char*, ltstr> strings;
	static bool hasPrefsFile;
};

template<class T>
unsigned PlayerPrefs::GetMapSize(const std::map<const char*, T, ltstr>& myMap)
{
	unsigned size = sizeof(unsigned);
	typename std::map<const char*, T, ltstr>::const_iterator it;
	for (it = myMap.begin(); it != myMap.end(); it++)
	{
		size += strlen(it->first) + 1 + sizeof(T);
	}
	return size;
}

template<>
inline unsigned PlayerPrefs::GetMapSize(const std::map<const char*, const char*, ltstr>& myMap)
{
	unsigned size = sizeof(unsigned);
	std::map<const char*, const char*, ltstr>::const_iterator it;
	for (it = myMap.begin(); it != myMap.end(); it++)
	{
		size += strlen(it->first) + 1 + strlen(it->second) + 1;
	}
	return size;
}

template <class T>
void PlayerPrefs::SerializeMap(const std::map<const char*, T, ltstr>& myMap, char** cursor)
{
	unsigned size = myMap.size();
	memcpy(*cursor, &size, sizeof(unsigned));
	*cursor += sizeof(unsigned);

	typename std::map<const char*, T, ltstr>::const_iterator it;
	for (it = myMap.begin(); it != myMap.end(); it++)
	{
		memcpy(*cursor, it->first, strlen(it->first) + 1);
		*cursor += strlen(it->first) + 1;
		SaveValue(cursor, it->second);
	}
}

template<class T>
inline void PlayerPrefs::SaveValue(char** cursor, T value)
{
	memcpy(*cursor, &value, sizeof(T));
	*cursor += sizeof(T);
}

template<>
inline void PlayerPrefs::SaveValue(char** cursor, const char* value)
{
	memcpy(*cursor, value, strlen(value)+1);
	*cursor += strlen(value)+1;
}

template <class T>
void PlayerPrefs::DeSerializeMap(std::map<const char*, T, ltstr>& myMap, char** cursor)
{
	unsigned size = 0u;
	memcpy(&size, *cursor, sizeof(unsigned));
	*cursor += sizeof(unsigned);

	for (size_t i = 0; i < size; i++)
	{
		unsigned keySize = strlen(*cursor) + 1;
		char* key = new char[keySize];
		memcpy(key, *cursor, keySize);
		*cursor += keySize;
		T value;
		DeSerializeValue(value, cursor);

		myMap.emplace(key, value);
	}
}

template<class T>
inline void PlayerPrefs::DeSerializeValue(T& value, char** cursor)
{
	memcpy(&value, *cursor, sizeof(T));
	*cursor += sizeof(T);
}


template<>
inline void PlayerPrefs::DeSerializeValue(const char*& value, char** cursor)
{
	value = new char[strlen(*cursor) +1];
	memcpy((char*)value, *cursor, strlen(*cursor) + 1);
	*cursor += strlen(*cursor) + 1;
}
template<class T>
T PlayerPrefs::Get(const std::map<const char*, T, ltstr>& myMap, const char* key, T defaultValue)
{
	assert(key != nullptr);
	typename std::map<const char*, T, ltstr>::const_iterator it = myMap.find(key);
	if (it != myMap.end())
	{
		return it->second;
	}
	return defaultValue;
}

template<class T>
inline void PlayerPrefs::Set(std::map<const char*, T, ltstr>& myMap, const char* key, T value)
{
	DeleteKey(key);
	assert(key != nullptr);
	char* cpyKey = new char[strlen(key) + 1];
	memcpy(cpyKey, key, strlen(key) + 1);
	myMap.emplace(cpyKey, value);
}

template<class T>
void PlayerPrefs::CleanMap(std::map<const char*, T, ltstr>& myMap)
{
	typename std::map<const char*, T, ltstr>::iterator it;
	for (it = myMap.begin(); it != myMap.end(); it++)
	{
		ClearPosition(it->first, it->second);
	}
	myMap.clear();
}

template<class T>
inline void PlayerPrefs::ClearPosition(const char* key, T & value)
{
	delete[] key;
}

template<>
inline void PlayerPrefs::ClearPosition(const char* key, const char*& value)
{
	delete[] key;
	delete[] value;
}

template<class T>
bool PlayerPrefs::Find(const std::map<const char*, T, ltstr>& myMap, const char* key)
{
	typename std::map<const char*, T, ltstr>::const_iterator it = myMap.find(key);
	return it != myMap.end();
}

template<class T>
typename std::map<const char*, T, ltstr>::const_iterator PlayerPrefs::FindIter(const std::map<const char*, T, ltstr>& myMap, const char* key)
{
	return myMap.find(key);
}

template<class T>
bool PlayerPrefs::Delete(std::map<const char*, T, ltstr>& myMap, const char* key)
{
	typename std::map<const char*, T, ltstr>::const_iterator it = FindIter(myMap, key);
	if (it != myMap.end())
	{
		myMap.erase(it);
		return true;
	}
	return false;
}

#endif __PLAYERPREFS_H__
