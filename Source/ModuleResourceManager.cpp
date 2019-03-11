#include "ModuleResourceManager.h"

#include "Application.h"
#include "ModuleTextures.h"
#include "ModuleProgram.h"
#include "ModuleRender.h"

#include "Material.h"
#include "Mesh.h"

ModuleResourceManager::ModuleResourceManager()
{
}


ModuleResourceManager::~ModuleResourceManager()
{
}


Texture* ModuleResourceManager::GetTexture(std::string filename) const
{
	std::map<std::string, std::pair<unsigned, Texture*>>::const_iterator it = textureResources.find(filename);
	if (it != textureResources.end())
	{
		return it->second.second;
	}
	return nullptr;
}

void ModuleResourceManager::AddTexture(Texture* texture)
{
	std::map<std::string, std::pair<unsigned, Texture*>>::iterator it = textureResources.find(texture->file);
	if (it != textureResources.end())
	{
		it->second.first++;
	}
	else
	{
		textureResources.insert(std::pair<std::string, std::pair<unsigned, Texture*>>
			(texture->file, std::pair<unsigned, Texture*>(1,texture)));
	}
}

void ModuleResourceManager::DeleteTexture(std::string filename)
{
	std::map<std::string, std::pair<unsigned, Texture*>>::iterator it = textureResources.find(filename);
	if (it != textureResources.end())
	{
		if (it->second.first > 1)
		{
			it->second.first--;
		}
		else
		{
			RELEASE(it->second.second);
			textureResources.erase(it);
		}
	}
}

Shader* ModuleResourceManager::GetProgram(std::string filename) const
{
	std::map<std::string, std::pair<unsigned, Shader*>>::const_iterator it = shaderResources.find(filename);
	if (it != shaderResources.end())
	{
		return it->second.second;
	}
	return nullptr;
}

std::list<Shader*> ModuleResourceManager::GetAllPrograms() const
{
	std::list<Shader*> programlist;
	for (const auto & resource : shaderResources)
	{
		programlist.push_back(resource.second.second);
	}
	return programlist;
}

void ModuleResourceManager::AddProgram(Shader* shader)
{
	std::map<std::string, std::pair<unsigned, Shader*>>::iterator it = shaderResources.find(shader->file);
	if (it != shaderResources.end())
	{
		it->second.first++;
	}
	else
	{
		shaderResources.insert(std::pair<std::string, std::pair<unsigned, Shader*>>
			(shader->file, std::pair<unsigned, Shader*>(1, shader)));
		App->renderer->AddBlockUniforms(*shader); 
	}
}

void ModuleResourceManager::DeleteProgram(std::string filename)
{
	std::map<std::string, std::pair<unsigned, Shader*>>::iterator it = shaderResources.find(filename);
	if (it != shaderResources.end())
	{
		if (it->second.first > 1)
		{
			it->second.first--;
		}
		else
		{
			RELEASE(it->second.second);
			shaderResources.erase(it);
		}
	}
}

Material * ModuleResourceManager::GetMaterial(std::string filename) const
{
	std::map<std::string, std::pair<unsigned, Material*>>::const_iterator it = materialResources.find(filename);
	if (it != materialResources.end())
	{
		return it->second.second;
	}
	return nullptr;
}

void ModuleResourceManager::AddMaterial(Material * material)
{
	std::map<std::string, std::pair<unsigned, Material*>>::iterator it = materialResources.find(material->name);
	if (it != materialResources.end())
	{
		it->second.first++;
	}
	else
	{
		materialResources.insert(std::pair<std::string, std::pair<unsigned, Material*>>
			(material->name, std::pair<unsigned, Material*>(1, material)));
	}
}

void ModuleResourceManager::DeleteMaterial(std::string filename)
{
	std::map<std::string, std::pair<unsigned, Material*>>::iterator it = materialResources.find(filename);
	if (it != materialResources.end())
	{
		if (it->second.first > 1)
		{
			it->second.first--;
		}
		else
		{
			RELEASE(it->second.second);
			materialResources.erase(it);
		}
	}
}

Mesh * ModuleResourceManager::GetMesh(unsigned uid) const
{
	std::map<unsigned, std::pair<unsigned, Mesh*>>::const_iterator it = meshResources.find(uid);
	if (it != meshResources.end())
	{
		return it->second.second;
	}
	return nullptr;
}

void ModuleResourceManager::AddMesh(Mesh * mesh)
{
	std::map<unsigned, std::pair<unsigned, Mesh*>>::iterator it = meshResources.find(mesh->UID);
	if (it != meshResources.end())
	{
		it->second.first++;
	}
	else
	{
		meshResources.insert(std::pair<unsigned, std::pair<unsigned, Mesh*>>
			(mesh->UID, std::pair<unsigned, Mesh*>(1, mesh)));
	}
}

void ModuleResourceManager::DeleteMesh(unsigned uid)
{
	std::map<unsigned, std::pair<unsigned, Mesh*>>::iterator it = meshResources.find(uid);
	if (it != meshResources.end())
	{
		if (it->second.first > 1)
		{
			it->second.first--;
		}
		else
		{
			RELEASE(it->second.second);
			meshResources.erase(it);
		}
	}
}
