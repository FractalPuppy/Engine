#include "ResourceMaterial.h"

#include "Globals.h"
#include "Application.h"
#include "ModuleProgram.h"
#include "ModuleTextures.h"
#include "ModuleResourceManager.h"
#include "ModuleFileSystem.h"

#include "ResourceTexture.h"

#include "JSON.h"
#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
#include "GL/glew.h"

ResourceMaterial::ResourceMaterial(unsigned uid) : Resource(uid, TYPE::MATERIAL)
{
}

ResourceMaterial::ResourceMaterial(const ResourceMaterial& resource) : Resource(resource)
{
	name = resource.name;
	if (resource.shader != nullptr)
	{
		shader = App->program->GetProgram(resource.shader->file.c_str());
	}
	for (unsigned i = 0; i < MAXTEXTURES; ++i)
	{
		if (resource.textures[i] != nullptr)
		{
			textures[i] = (ResourceTexture*)App->resManager->Get(resource.textures[i]->GetExportedFile());
		}
	}

	diffuseColor = resource.diffuseColor;
	specularColor = resource.specularColor;
	emissiveColor = resource.emissiveColor;

	roughness = resource.roughness;
	metallic = resource.metallic;
}

ResourceMaterial::~ResourceMaterial()
{
	DeleteFromMemory();
}

void ResourceMaterial::DeleteFromMemory()
{
	Resource::DeleteFromMemory();
	if (shader != nullptr)
	{
		App->resManager->DeleteProgram(shader->file);
		shader = nullptr;
	}
	for (unsigned i = 0; i < MAXTEXTURES; i++)
	{
		if (textures[i] != nullptr)
		{
			App->resManager->DeleteResource(textures[i]->GetUID());
			textures[i] = nullptr;
		}
	}
}

bool ResourceMaterial::LoadInMemory()
{
	char* data = nullptr;
	// Load JSON
	if (App->fsystem->Load((IMPORTED_MATERIALS + exportedFileName + MATERIALEXT).c_str(), &data) == 0)
		return false;

	JSON* json = new JSON(data);
	JSON_value* materialJSON = json->GetValue("material");

	name = exportedFileName;
	diffuseColor = materialJSON->GetColor4("diffuseColor");
	specularColor = materialJSON->GetColor3("specularColor");
	emissiveColor = materialJSON->GetColor3("emissiveColor");

	metallic = materialJSON->GetFloat("metallic");
	roughness = materialJSON->GetFloat("roughness");

	const char* diffuseFile = materialJSON->GetString("diffuse");
	if (diffuseFile != nullptr)
	{
		textures[(unsigned)TextureType::DIFFUSE] = (ResourceTexture*)App->resManager->Get(diffuseFile);
	}
	const char* specularFile = materialJSON->GetString("specular");
	if (specularFile != nullptr)
	{
		textures[(unsigned)TextureType::SPECULAR] = (ResourceTexture*)App->resManager->Get(specularFile);
	}
	const char* occlusionFile = materialJSON->GetString("occlusion");
	if (occlusionFile != nullptr)
	{
		textures[(unsigned)TextureType::OCCLUSION] = (ResourceTexture*)App->resManager->Get(occlusionFile);
	}
	const char* emissiveFile = materialJSON->GetString("emissive");
	if (emissiveFile != nullptr)
	{
		textures[(unsigned)TextureType::EMISSIVE] = (ResourceTexture*)App->resManager->Get(emissiveFile);
	}
	const char* normalFile = materialJSON->GetString("normal");
	if (normalFile != nullptr)
	{
		textures[(unsigned)TextureType::NORMAL] = (ResourceTexture*)App->resManager->Get(normalFile);
	}

	const char* shaderName = materialJSON->GetString("shader");
	if (shaderName != nullptr)
	{
		shader = App->program->GetProgram(materialJSON->GetString("shader"));
	}

	RELEASE_ARRAY(data);
	RELEASE(json);
	++loaded;
	return true;
}

void ResourceMaterial::Save() const
{
	JSON* json = new JSON();
	JSON_value* materialJSON = json->CreateValue();

	if (textures[(unsigned)TextureType::DIFFUSE] != nullptr)
		materialJSON->AddFloat4("diffuseColor", diffuseColor);

	if (textures[(unsigned)TextureType::SPECULAR] != nullptr)
		materialJSON->AddFloat3("specularColor", specularColor);

	if (textures[(unsigned)TextureType::EMISSIVE] != nullptr)
		materialJSON->AddFloat3("emissiveColor", emissiveColor);

	materialJSON->AddFloat("metallic", metallic);
	materialJSON->AddFloat("roughness", roughness);


	if (textures[(unsigned)TextureType::DIFFUSE] != nullptr)
	{
		materialJSON->AddString("diffuse", textures[(unsigned)TextureType::DIFFUSE]->GetExportedFile());
	}
	if (textures[(unsigned)TextureType::SPECULAR] != nullptr)
	{
		materialJSON->AddString("specular", textures[(unsigned)TextureType::SPECULAR]->GetExportedFile());
	}
	if (textures[(unsigned)TextureType::OCCLUSION] != nullptr)
	{
		materialJSON->AddString("occlusion", textures[(unsigned)TextureType::OCCLUSION]->GetExportedFile());
	}
	if (textures[(unsigned)TextureType::EMISSIVE] != nullptr)
	{
		materialJSON->AddString("emissive", textures[(unsigned)TextureType::EMISSIVE]->GetExportedFile());
	}
	if (textures[(unsigned)TextureType::NORMAL] != nullptr)
	{
		materialJSON->AddString("normal", textures[(unsigned)TextureType::NORMAL]->GetExportedFile());
	}

	if (shader != nullptr)
	{
		materialJSON->AddString("shader", shader->file.c_str());
	}
	
	json->AddValue("material", *materialJSON);

	App->fsystem->Save((MATERIALS + name + MATERIALEXT).c_str(), json->ToString().c_str(), json->Size());
	RELEASE(json);
}

void ResourceMaterial::SaveMetafile(const char* file) const
{
	std::string filepath;
	filepath.append(file);
	JSON* json = new JSON();
	JSON_value* meta = json->CreateValue();
	struct stat statFile;
	stat(filepath.c_str(), &statFile);
	meta->AddUint("GUID", UID);
	meta->AddUint("timeCreated", statFile.st_ctime);
	filepath += ".meta";
	App->fsystem->Save(filepath.c_str(), json->ToString().c_str(), json->Size());
}

void ResourceMaterial::Reset(const ResourceMaterial& material)
{
	name = material.name;

	if (shader != nullptr)
	{
		App->resManager->DeleteProgram(shader->file);
	}
	if (material.shader != nullptr)
	{
		shader = App->program->GetProgram(material.shader->file.c_str());
	}
	for (unsigned i = 0; i < MAXTEXTURES; ++i)
	{
		if (textures[i] != nullptr)
		{
			App->resManager->DeleteResource(textures[i]->GetUID());
		}
		if (material.textures[i] != nullptr)
		{
			textures[i] = (ResourceTexture*)App->resManager->Get(material.textures[i]->GetExportedFile());
		}
		else
		{
			textures[i] = nullptr;
		}
	}
	diffuseColor = material.diffuseColor;
	specularColor = material.specularColor;
	emissiveColor = material.emissiveColor;

	roughness = material.roughness;
	metallic = material.metallic;
}

int ResourceMaterial::Compare(const ResourceMaterial& material)
{
	if (name.compare(material.name) != 0)
		return -1;

	if (shader != material.shader)
		return 0;

	for (unsigned i = 0; i < MAXTEXTURES; ++i)
	{
		if (textures[i] != material.textures[i])
			return 0;
	}

	if (!diffuseColor.Equals(material.diffuseColor))
		return 0;
	if (!specularColor.Equals(material.specularColor))
		return 0;
	if (!emissiveColor.Equals(material.emissiveColor))
		return 0;

	if (roughness != material.roughness)
		return 0;
	if (metallic != material.metallic)
		return 0;

	return 1;
}

ResourceTexture* ResourceMaterial::GetTexture(TextureType type) const
{
	return textures[(unsigned)type];
}

std::list<ResourceTexture*> ResourceMaterial::GetTextures() const
{
	std::list<ResourceTexture*> mytextures;
	for (unsigned i = 0; i < MAXTEXTURES; i++)
	{
		if (textures[i] != nullptr)
		{
			mytextures.push_back(textures[i]);
		}
	}
	return mytextures;
}

void ResourceMaterial::SetUniforms(unsigned shader) const
{
	for (unsigned int i = 0; i < MAXTEXTURES; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);

		char* textureType = nullptr;
		float* color = (float*)&float3::zero;
		switch ((TextureType)i)
		{
		case TextureType::DIFFUSE:
			textureType = "diffuse";
			color = (float*)&diffuseColor;
			break;

		case TextureType::SPECULAR:
			textureType = "specular";
			color = (float*)&specularColor;
			break;

		case TextureType::OCCLUSION:
			textureType = "occlusion";
			break;

		case TextureType::EMISSIVE:
			textureType = "emissive";
			color = (float*)&emissiveColor;
			break;
		case TextureType::NORMAL:
			textureType = "normal";
			break;
		}

		char texture[32];
		sprintf(texture, "material.%s_texture", textureType);

		char uniform[32];
		sprintf(uniform, "material.%s_color", textureType);

		if (textures[(int)TextureType::NORMAL] == nullptr)
		{
			glUniform1i(glGetUniformLocation(shader, "hasNormalMap"), 0);
		}
		else
		{
			glUniform1i(glGetUniformLocation(shader, "hasNormalMap"), 1);
		}

		if (textures[i] != nullptr)
		{
			if (i == (unsigned)TextureType::DIFFUSE)
			{
				glUniform4fv(glGetUniformLocation(shader,
					uniform), 1, color);
			}
			else
			{
				glUniform3fv(glGetUniformLocation(shader,
					uniform), 1, color);
			}
			glBindTexture(GL_TEXTURE_2D, textures[i]->gpuID);

			glUniform1i(glGetUniformLocation(shader, texture), i);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, 0);
			glUniform1i(glGetUniformLocation(shader, texture), i);
			float3 noColor = float3::zero; //Used as a fallback

			glUniform3fv(glGetUniformLocation(shader,
				uniform), 1, (GLfloat*)&noColor);
		}
	}

	glUniform1fv(glGetUniformLocation(shader,
		"material.roughness"), 1, (GLfloat*)&roughness);
	glUniform1fv(glGetUniformLocation(shader,
		"material.metallic"), 1, (GLfloat*)&metallic);
}