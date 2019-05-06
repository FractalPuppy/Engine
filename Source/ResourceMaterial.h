#ifndef __ResourceMaterial_h__
#define __ResourceMaterial_h__

#include "Resource.h"
#include "Math/float4.h"
#include <list>

#define MAXTEXTURES 5
#define DEFAULTMAT "Default"

enum class TextureType
{
	DIFFUSE = 0,
	SPECULAR,
	OCCLUSION,
	EMISSIVE,
	NORMAL
};

class ResourceTexture;
struct Shader;

class ResourceMaterial :
	public Resource
{
public:
	ResourceMaterial(unsigned uid);
	ResourceMaterial(const ResourceMaterial& resource);
	~ResourceMaterial();

	bool LoadInMemory() override;
	void DeleteFromMemory() override;
	void Save() const;										// Saves material file (.m4t) in Assets/Materials
	void SaveMetafile(const char* file) const override;
	void Reset(const ResourceMaterial& material);
	int Compare(const ResourceMaterial& material);

	// File in asset specific
	void Rename(const char* newName) override;
	void Delete() override;

	ResourceTexture* GetTexture(TextureType type) const;
	std::list<ResourceTexture*> GetTextures() const;
	void SetUniforms(unsigned shader) const;

public:
	Shader* shader = nullptr;

	ResourceTexture* textures[MAXTEXTURES]{ nullptr };

	math::float4 diffuseColor =  math::float4::one;
	math::float3 emissiveColor = math::float3::one;
	math::float3 specularColor = math::float3(.1f, .1f, .1f);

	float roughness = .5f;
	float metallic = .5f;

	unsigned variation = 0u;
};

#endif __ResourceMaterial_h__