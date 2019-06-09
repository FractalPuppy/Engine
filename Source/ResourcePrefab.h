#ifndef __Prefab_h__
#define __Prefab_h__

#include "Resource.h"

#include <vector>

class GameObject;
class JSON;
class JSON_value;

class ResourcePrefab : public Resource
{
public:
	ResourcePrefab(unsigned uid);
	ResourcePrefab(const ResourcePrefab& resource);
	virtual ~ResourcePrefab();

	bool LoadInMemory() override;
	void DeleteFromMemory() override;

	void SaveMetafile(const char* file) const override;
	void LoadConfigFromMeta() override;

	void AddInstance(GameObject * go);
	bool RemoveInstance(GameObject * go);
	unsigned GetNbInstances() const;

	void Update(GameObject* go);
	GameObject * RetrievePrefab();
	void Save(GameObject* go) const;

private:
	void Load(char** data);

private:
	std::vector<GameObject*> instances;
	JSON_value* prefabValue = nullptr;
	JSON* prefabJson = nullptr;

public:
	GameObject* root = nullptr; //version of prefab on starting engine

};

#endif __ResourcePrefab_h__