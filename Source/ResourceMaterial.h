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
	void Save() const;	// Saves material file (.m4t) in Assets/Materials
	void SaveMetafile(const char* file) const override;
	void Reset(const ResourceMaterial& material);
	int Compare(const ResourceMaterial& material);

	ResourceTexture* GetTexture(TextureType type) const;
	std::list<ResourceTexture*> GetTextures() const;
	void SetUniforms(unsigned shader) const;

public:
	std::string name;
	Shader* shader = nullptr;

	ResourceTexture* textures[MAXTEXTURES]{ nullptr };

	float4 diffuseColor = float4::one;
	float3 specularColor = float3::one;
	float3 emissiveColor = float3::one;

	float roughness = .5f;
	float metallic = .5f;
};

#endif __ResourceMaterial_h__