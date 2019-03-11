
#include "Application.h"

#include "ModuleFileSystem.h"
#include "ModuleTextures.h"
#include "ModuleProgram.h"
#include "ModuleResourceManager.h"

#include "Material.h"

#include "Globals.h"
#include "JSON.h"
#include "GL/glew.h"

Material::Material()
{
	shader = App->program->GetProgram(DEFAULTPROGRAM);
}

Material::Material(const char* name)
{
	this->name = name;
	shader = App->program->GetProgram(DEFAULTPROGRAM);
}

Material::Material(const Material& material)
{
	name = material.name;
	if (material.shader != nullptr)
	{
		shader = App->program->GetProgram(material.shader->file.c_str());
	}
	for (unsigned i = 0; i < MAXTEXTURES; ++i)
	{
		if (material.textures[i] != nullptr)
		{
			textures[i] = App->textures->GetTexture(material.textures[i]->file.c_str());
		}
	}

	diffuseColor = material.diffuseColor;
	specularColor = material.specularColor;
	emissiveColor = material.emissiveColor;

	kAmbient = material.kAmbient;
	kDiffuse = material.kDiffuse;
	kSpecular = material.kSpecular;
	shininess = material.shininess;
}

Material::~Material()
{
	/*if (changesDone)
	{
		Save();
	}*/

	if (shader != nullptr)
	{
		App->resManager->DeleteProgram(shader->file);
		shader = nullptr;
	}
	for (unsigned i = 0; i < MAXTEXTURES; i++)
	{
		if (textures[i] != nullptr)
		{
			App->resManager->DeleteTexture(textures[i]->file);
			textures[i] = nullptr;
		}
	}
}

void Material::Load(const char * materialfile)
{
	char* data = nullptr;
	std::string materialName(materialfile);
	App->fsystem->Load((MATERIALS + materialName + JSONEXT).c_str(), &data);
	JSON *json = new JSON(data);
	JSON_value *materialJSON = json->GetValue("material");
	
	name = materialName;
	diffuseColor = materialJSON->GetColor4("diffuseColor");
	specularColor = materialJSON->GetColor3("specularColor");
	emissiveColor = materialJSON->GetColor3("emissiveColor");
	
	metallic = materialJSON->GetFloat("metallic");
	roughness = materialJSON->GetFloat("roughness");
	
	const char* diffuseFile = materialJSON->GetString("diffuse");
	if (diffuseFile != nullptr)
	{
		textures[(unsigned)TextureType::DIFFUSE] = App->textures->GetTexture(diffuseFile);
	}
	const char* specularFile = materialJSON->GetString("specular");
	if (specularFile != nullptr)
	{
		textures[(unsigned)TextureType::SPECULAR] = App->textures->GetTexture(specularFile);
	}
	const char* occlusionFile = materialJSON->GetString("occlusion");
	if (occlusionFile != nullptr)
	{
		textures[(unsigned)TextureType::OCCLUSION] = App->textures->GetTexture(occlusionFile);
	}
	const char* emissiveFile = materialJSON->GetString("emissive");
	if (emissiveFile != nullptr)
	{
		textures[(unsigned)TextureType::EMISSIVE] = App->textures->GetTexture(emissiveFile);
	}
	const char* normalFile = materialJSON->GetString("normal");
	if (normalFile != nullptr)
	{
		textures[(unsigned)TextureType::NORMAL] = App->textures->GetTexture(normalFile);
	}
	
	const char* shaderName = materialJSON->GetString("shader");
	if (shaderName != nullptr)
	{
		shader = App->program->GetProgram(materialJSON->GetString("shader"));
	}

	RELEASE_ARRAY(data);
	RELEASE(json);
}

void Material::Save() const
{
	JSON *json = new JSON();
	JSON_value *materialJSON = json->CreateValue();
	
	if(textures[(unsigned) TextureType::DIFFUSE] != nullptr)
		materialJSON->AddFloat4("diffuseColor", diffuseColor);
	
	if (textures[(unsigned)TextureType::SPECULAR] != nullptr)
		materialJSON->AddFloat3("specularColor", specularColor);
	
	if (textures[(unsigned)TextureType::EMISSIVE] != nullptr)
		materialJSON->AddFloat3("emissiveColor", emissiveColor);
	
	materialJSON->AddFloat("metallic", metallic);
	materialJSON->AddFloat("roughness", roughness);
		
	
	if (textures[(unsigned)TextureType::DIFFUSE] != nullptr)
	{
		materialJSON->AddString("diffuse", textures[(unsigned)TextureType::DIFFUSE]->file.c_str());
	}
	if (textures[(unsigned)TextureType::SPECULAR] != nullptr)
	{
		materialJSON->AddString("specular", textures[(unsigned)TextureType::SPECULAR]->file.c_str());
	}
	if (textures[(unsigned)TextureType::OCCLUSION] != nullptr)
	{
		materialJSON->AddString("occlusion", textures[(unsigned)TextureType::OCCLUSION]->file.c_str());
	}
	if (textures[(unsigned)TextureType::EMISSIVE] != nullptr)
	{
		materialJSON->AddString("emissive", textures[(unsigned)TextureType::EMISSIVE]->file.c_str());
	}
	if (textures[(unsigned)TextureType::NORMAL] != nullptr)
	{
		materialJSON->AddString("normal", textures[(unsigned)TextureType::NORMAL]->file.c_str());
	}
	
	if (shader != nullptr)
	{
		materialJSON->AddString("shader", shader->file.c_str());
	}
	
	json->AddValue("material", *materialJSON);
	
	App->fsystem->Save((MATERIALS + name + JSONEXT).c_str(), json->ToString().c_str(),json->Size());
	RELEASE(json);
}

void Material::Reset(const Material & material)
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
			App->resManager->DeleteTexture(textures[i]->file);
		}
		if (material.textures[i] != nullptr)
		{
			textures[i] = App->textures->GetTexture(material.textures[i]->file.c_str());
		}
		else
		{
			textures[i] = nullptr;
		}
	}
	diffuseColor = material.diffuseColor;
	specularColor = material.specularColor;
	emissiveColor = material.emissiveColor;

	kAmbient = material.kAmbient;
	kDiffuse = material.kDiffuse;
	kSpecular = material.kSpecular;
	shininess = material.shininess;
}

int Material::Compare(const Material& material)
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

	if (kDiffuse != material.kDiffuse)
		return 0;
	if (kSpecular != material.kSpecular)
		return 0;
	if (kAmbient != material.kAmbient)
		return 0;

	if (shininess != material.shininess)
		return 0;

	return 1;
}

Texture * Material::GetTexture(TextureType type) const
{
	return textures[(unsigned)type];
}

std::list<Texture*> Material::GetTextures() const
{
	std::list<Texture*> mytextures;
	for (unsigned i = 0; i < MAXTEXTURES; i++)
	{
		if (textures[i] != nullptr)
		{
			mytextures.push_back(textures[i]);
		}
	}
	return mytextures;
}

void Material::SetUniforms(unsigned shader) const
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
			glBindTexture(GL_TEXTURE_2D, textures[i]->id);

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
