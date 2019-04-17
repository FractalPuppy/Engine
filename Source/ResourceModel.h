#ifndef __ResourceModel_h__
#define __ResourceModel_h__

#include "Resource.h"

#include <vector>

class ResourceMesh;
class ResourceAnimation;

class ResourceModel :
	public Resource
{
public:
	ResourceModel(unsigned uid);
	ResourceModel(const ResourceModel& resource);
	~ResourceModel();

	bool LoadInMemory() override;		// Will load in memory all ResourceMeshes stored
	void DeleteFromMemory() override;	// Will delete all ResourceMeshes stored in memory

	// File in asset specific
	void Rename(const char* newName) override;
	void Delete() override;

	void SaveMetafile(const char* file) const override;
	void LoadConfigFromMeta() override;
	bool CheckImportedMeshes();					// False if all meshes inside the model had been imported, true if not
	bool CheckImportedAnimations();					// False if all meshes inside the model had been imported, true if not

	void AddMesh(ResourceMesh* mesh);			// Adds a resource mesh to the meshList, if another resource mesh had the same UID it gets replaced
	void AddAnimation(ResourceAnimation* anim);
	void DrawImportConfiguration() override;

public:
	unsigned numMeshes = 0u;
	std::vector<ResourceMesh*> meshList;

	unsigned numAnimations = 0u;
	std::vector<ResourceAnimation*> animationList;
};

#endif __ResourceModel_h__