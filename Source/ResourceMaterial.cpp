#include "ResourceMaterial.h"

#include "Globals.h"
#include "Application.h"
#include "ModuleProgram.h"
#include "ModuleTextures.h"
#include "ModuleResourceManager.h"
#include "ModuleFileSystem.h"
#include "ModuleTime.h"
#include "ComponentRenderer.h"

#include "ResourceTexture.h"

#include "JSON.h"
#include "GL/glew.h"

ResourceMaterial::ResourceMaterial(unsigned uid) : Resource(uid, TYPE::MATERIAL)
{
}

ResourceMaterial::ResourceMaterial(const ResourceMaterial& resource) : Resource(resource)
{
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
	bloomIntenstiy = resource.bloomIntenstiy;
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
	if (App->fsystem->Load(exportedFile.c_str(), &data) == 0)
		return false;

	JSON* json = new JSON(data);
	JSON_value* materialJSON = json->GetValue("material");

	diffuseColor = materialJSON->GetColor4("diffuseColor");
	specularColor = materialJSON->GetColor3("specularColor");
	emissiveColor = materialJSON->GetColor3("emissiveColor");
	dissolveColor = materialJSON->GetColor3("dissolveColor");

	bloomIntenstiy = materialJSON->GetFloat("bloomIntenstiy", bloomIntenstiy);
	roughness = materialJSON->GetFloat("roughness");

	unsigned diffuseUID = materialJSON->GetUint("diffuseUID");
	if (diffuseUID != 0u)
	{
		textures[(unsigned)TextureType::DIFFUSE] = (ResourceTexture*)App->resManager->Get(diffuseUID);
	}
	unsigned specularUID = materialJSON->GetUint("specularUID");
	if (specularUID != 0u)
	{
		textures[(unsigned)TextureType::SPECULAR] = (ResourceTexture*)App->resManager->Get(specularUID);
	}
	unsigned occlusionUID = materialJSON->GetUint("occlusionUID");
	if (occlusionUID != 0u)
	{
		textures[(unsigned)TextureType::OCCLUSION] = (ResourceTexture*)App->resManager->Get(occlusionUID);
	}
	unsigned emissiveUID = materialJSON->GetUint("emissiveUID");
	if (emissiveUID != 0u)
	{
		textures[(unsigned)TextureType::EMISSIVE] = (ResourceTexture*)App->resManager->Get(emissiveUID);
	}
	unsigned normalUID = materialJSON->GetUint("normalUID");
	if (normalUID != 0u)
	{
		textures[(unsigned)TextureType::NORMAL] = (ResourceTexture*)App->resManager->Get(normalUID);
	}
	unsigned dissolveUID = materialJSON->GetUint("dissolveUID");
	if (dissolveUID != 0u)
	{
		textures[(unsigned)TextureType::DISSOLVE] = (ResourceTexture*)App->resManager->Get(dissolveUID);
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

	materialJSON->AddFloat("bloomIntenstiy", bloomIntenstiy);
	materialJSON->AddFloat("roughness", roughness);
	materialJSON->AddFloat3("specularColor", specularColor);
	materialJSON->AddFloat3("emissiveColor", emissiveColor);
	materialJSON->AddFloat3("dissolveColor", dissolveColor);


	if (textures[(unsigned)TextureType::DIFFUSE] != nullptr)
	{
		materialJSON->AddUint("diffuseUID", textures[(unsigned)TextureType::DIFFUSE]->GetUID());
	}
	if (textures[(unsigned)TextureType::SPECULAR] != nullptr)
	{
		materialJSON->AddUint("specularUID", textures[(unsigned)TextureType::SPECULAR]->GetUID());
	}
	if (textures[(unsigned)TextureType::OCCLUSION] != nullptr)
	{
		materialJSON->AddUint("occlusionUID", textures[(unsigned)TextureType::OCCLUSION]->GetUID());
	}
	if (textures[(unsigned)TextureType::EMISSIVE] != nullptr)
	{
		materialJSON->AddUint("emissiveUID", textures[(unsigned)TextureType::EMISSIVE]->GetUID());
	}
	if (textures[(unsigned)TextureType::NORMAL] != nullptr)
	{
		materialJSON->AddUint("normalUID", textures[(unsigned)TextureType::NORMAL]->GetUID());
	}
	if (textures[(unsigned)TextureType::DISSOLVE] != nullptr)
	{
		materialJSON->AddUint("dissolveUID", textures[(unsigned)TextureType::DISSOLVE]->GetUID());
	}

	if (shader != nullptr)
	{
		materialJSON->AddString("shader", shader->file.c_str());
	}
	
	json->AddValue("material", *materialJSON);

	App->fsystem->Save(file.c_str(), json->ToString().c_str(), json->Size());
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
	meta->AddUint("metaVersion", META_VERSION);
	meta->AddUint("timeCreated", statFile.st_ctime);

	// Resource info
	meta->AddUint("GUID", UID);
	meta->AddString("Name", name.c_str());
	meta->AddString("File", file);
	meta->AddString("ExportedFile", exportedFile.c_str());

	meta->AddFloat4("DifusseColor", diffuseColor);
	meta->AddFloat3("specularColor", specularColor);
	meta->AddFloat3("emissiveColor", emissiveColor);
	if (textures[(unsigned)TextureType::DIFFUSE] != nullptr)
	{
		meta->AddUint("diffuseUID", textures[(unsigned)TextureType::DIFFUSE]->GetUID());
	}
	if (textures[(unsigned)TextureType::SPECULAR] != nullptr)
	{
		meta->AddUint("specularUID", textures[(unsigned)TextureType::SPECULAR]->GetUID());
	}
	if (textures[(unsigned)TextureType::OCCLUSION] != nullptr)
	{
		meta->AddUint("occlusionUID", textures[(unsigned)TextureType::OCCLUSION]->GetUID());
	}
	if (textures[(unsigned)TextureType::EMISSIVE] != nullptr)
	{
		meta->AddUint("emissiveUID", textures[(unsigned)TextureType::EMISSIVE]->GetUID());
	}
	if (textures[(unsigned)TextureType::NORMAL] != nullptr)
	{
		meta->AddUint("normalUID", textures[(unsigned)TextureType::NORMAL]->GetUID());
	}
	if (textures[(unsigned)TextureType::DISSOLVE] != nullptr)
	{
		meta->AddUint("dissolveUID", textures[(unsigned)TextureType::DISSOLVE]->GetUID());
	}

	if (shader != nullptr)
	{
		meta->AddString("shader", shader->file.c_str());
	}
	json->AddValue("Material", *meta);
	filepath += METAEXT;

	// Save meta in Assets
	App->fsystem->Save(filepath.c_str(), json->ToString().c_str(), json->Size());

	// Save meta in Library
	std::string libraryPath(exportedFile + METAEXT);
	App->fsystem->Save(libraryPath.c_str(), json->ToString().c_str(), json->Size());
	RELEASE(json);
}

void ResourceMaterial::LoadConfigFromMeta()
{
	std::string metaFile(file);
	metaFile += ".meta";

	// Check if meta file exists
	if (!App->fsystem->Exists(metaFile.c_str()))
		return;

	char* data = nullptr;
	unsigned oldUID = GetUID();

	if (App->fsystem->Load(metaFile.c_str(), &data) == 0)
	{
		LOG("Warning: %s couldn't be loaded", metaFile.c_str());
		RELEASE_ARRAY(data);
		return;
	}
	JSON* json = new JSON(data);
	JSON_value* value = json->GetValue("Material");

	// Make sure the UID from meta is the same
	unsigned checkUID = value->GetUint("GUID");
	if (oldUID != checkUID)
	{
		UID = checkUID;
		// Update resource UID on resource list
		App->resManager->ReplaceResource(oldUID, this);
		exportedFile = IMPORTED_MATERIALS + std::to_string(UID) + MATERIALEXT;
	}

	// Check the meta file version
	if (value->GetUint("metaVersion", 0u) < META_VERSION)
		SaveMetafile(file.c_str());

	// Check the meta saved in library, if not save it
	if (!App->fsystem->Exists((exportedFile + METAEXT).c_str()))
		SaveMetafile(file.c_str());

	RELEASE_ARRAY(data);
	RELEASE(json);
}

void ResourceMaterial::LoadConfigFromLibraryMeta()
{
	std::string metaFile(exportedFile);
	metaFile += ".meta";

	// Check if meta file exists
	if (!App->fsystem->Exists(metaFile.c_str()))
		return;

	char* data = nullptr;
	unsigned oldUID = GetUID();

	if (App->fsystem->Load(metaFile.c_str(), &data) == 0)
	{
		LOG("Warning: %s couldn't be loaded", metaFile.c_str());
		RELEASE_ARRAY(data);
		return;
	}
	JSON* json = new JSON(data);
	JSON_value* value = json->GetValue("Material");

	// Get resource variables
	name = value->GetString("Name");
	file = value->GetString("File");

	RELEASE_ARRAY(data);
	RELEASE(json);
}

void ResourceMaterial::Reset(const ResourceMaterial& material)
{
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
	bloomIntenstiy = material.bloomIntenstiy;
}

int ResourceMaterial::Compare(const ResourceMaterial& material)
{
	if (shader != material.shader)
		return false;

	for (unsigned i = 0; i < MAXTEXTURES; ++i)
	{
		if (textures[i] != material.textures[i])
			return false;
	}

	if (!diffuseColor.Equals(material.diffuseColor))
		return false;
	if (!specularColor.Equals(material.specularColor))
		return false;
	if (!emissiveColor.Equals(material.emissiveColor))
		return false;

	if (roughness != material.roughness)
		return false;
	if (bloomIntenstiy != material.bloomIntenstiy)
		return false;
	return true;
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

void ResourceMaterial::SetUniforms(unsigned shader, bool isFx, ComponentRenderer* cRenderer) const
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
		case TextureType::DISSOLVE:
			textureType = "dissolve";
			color = (float*)&dissolveColor;
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
	if (isFx)
	{
		cRenderer->Update(); 
		glUniform1i(glGetUniformLocation(shader, "xTiles"), cRenderer->xTiles);
		glUniform1i(glGetUniformLocation(shader, "yTiles"), cRenderer->yTiles);
		glUniform1i(glGetUniformLocation(shader, "f1Xpos"), cRenderer->f1Xpos);
		glUniform1i(glGetUniformLocation(shader, "f1Ypos"), cRenderer->f1Ypos);
		glUniform1i(glGetUniformLocation(shader, "f2Xpos"), cRenderer->f2Xpos);
		glUniform1i(glGetUniformLocation(shader, "f2Ypos"), cRenderer->f2Ypos);
		glUniform1f(glGetUniformLocation(shader, "mixAmount"), cRenderer->frameMix);		
		glUniform1f(glGetUniformLocation(shader, "bloomIntensity"), cRenderer->material->bloomIntenstiy);		
		glUniform1f(glGetUniformLocation(shader, "time"), App->time->realTime);
		glUniform2fv(glGetUniformLocation(shader, "uvSpeed"), 1, &cRenderer->texSpeed[0]);
		glUniform4fv(glGetUniformLocation(shader, "colorIn"), 1, &cRenderer->material->diffuseColor[0]);
	}
	else
	{
		glUniform1fv(glGetUniformLocation(shader,
			"material.roughness"), 1, (GLfloat*)&roughness);
		glUniform3fv(glGetUniformLocation(shader,
			"material.specular"), 1, (GLfloat*)&specularColor);
		glUniform1fv(glGetUniformLocation(shader,
			"material.bloomIntensity"), 1, (GLfloat*)&bloomIntenstiy);
	}
}
