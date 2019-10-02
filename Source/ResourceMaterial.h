#ifndef __ResourceMaterial_h__
#define __ResourceMaterial_h__

#include "Resource.h"
#include "Math/float4.h"
#include <list>

#define MAXTEXTURES 6
#define DEFAULTMAT "Default"

enum class TextureType
{
	DIFFUSE = 0,
	SPECULAR,
	OCCLUSION,
	EMISSIVE,
	NORMAL,
	DISSOLVE
};

class ResourceTexture;
class ComponentRenderer; 
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
	void LoadConfigFromMeta() override;
	void LoadConfigFromLibraryMeta() override;
	void Reset(const ResourceMaterial& material);
	int Compare(const ResourceMaterial& material);

	ResourceTexture* GetTexture(TextureType type) const;
	std::list<ResourceTexture*> GetTextures() const;
	void SetUniforms(unsigned shader, bool isFx, ComponentRenderer* cRenderer) const;

public:
	Shader* shader = nullptr;

	ResourceTexture* textures[MAXTEXTURES]{ nullptr };

	math::float4 diffuseColor =  math::float4::one;
	math::float3 emissiveColor = math::float3::one;
	math::float3 specularColor = math::float3(.1f, .1f, .1f);
	math::float3 dissolveColor = math::float3::one;

	float roughness = .5f;
	float bloomIntenstiy = 1.0f;

	unsigned variation = 0u;
};

#endif __ResourceMaterial_h__