#ifndef __ResourceSkybox_h__
#define __ResourceSkybox_h__

#include "Resource.h"
#include "Geometry/Frustum.h"
#define NUMFACES 6

struct Shader;
class ResourceTexture;

class ResourceSkybox :
	public Resource
{
public:
	ResourceSkybox(unsigned uid);
	ResourceSkybox(const ResourceSkybox& resource);
	~ResourceSkybox();

	bool LoadInMemory() override;
	void DeleteFromMemory() override;

	void LoadConfigFromMeta() override {};

	void SetTextures(std::string faces[NUMFACES]);
	void Draw(const Frustum& frustum, bool isEditor = false) const;

	unsigned GetCubemap() { return skybox_cubemap; };
	unsigned GetVAO() { return skyboxVAO; };
	unsigned GetVBO() { return skyboxVBO; };

private:
	unsigned LoadCubeMap(ResourceTexture* faces[]) const;

public:
	bool enabled = true;
	ResourceTexture* textures[NUMFACES]{ nullptr };
	Shader* shader = nullptr; //Deallocated in ResManager

private:
	unsigned skybox_cubemap = 0;
	unsigned skyboxVAO = 0;
	unsigned skyboxVBO = 0;
};

#endif __ResourceSkybox_h__