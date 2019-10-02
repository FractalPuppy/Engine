#ifndef __ResourceScene_h__
#define __ResourceScene_h__

#include "Resource.h"

class GameObject;

class ResourceScene :
	public Resource
{
public:
	ResourceScene(unsigned uid);
	ResourceScene(const ResourceScene& resource);
	virtual ~ResourceScene();

	void SaveMetafile(const char* file) const override;
	void LoadConfigFromMeta() override;
	void LoadConfigFromLibraryMeta() override;

	// Scene especific
	void Save(const GameObject &rootGO, bool selected = false);
	bool Load();

private:
	bool PrefabWasUpdated(GameObject* prefabGo) const;
	void AssignNewUUID(GameObject* go, unsigned UID);
};

#endif