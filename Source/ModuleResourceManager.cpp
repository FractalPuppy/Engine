#include "ModuleResourceManager.h"

#include "Globals.h"
#include "Application.h"
#include "ModuleTextures.h"
#include "ModuleProgram.h"
#include "ModuleRender.h"
#include "ModuleScene.h"
#include "ModuleFileSystem.h"

#include "Resource.h"
#include "ResourceTexture.h"
#include "ResourceMesh.h"
#include "ResourceMaterial.h"
#include "ResourceSkybox.h"

#include "FileImporter.h"

ModuleResourceManager::ModuleResourceManager()
{
}


ModuleResourceManager::~ModuleResourceManager()
{
}

bool ModuleResourceManager::Init(JSON * config)
{
	LoadEngineResources();
	return true;
}

bool ModuleResourceManager::Start()
{
	//TODO: Read metafiles from Assets/ instead and import or add to resources
	std::vector<std::string> files;
	std::vector<std::string> dirs;
	App->fsystem->ListFolderContent(MESHES, files, dirs);
	for each (std::string file in files)
	{
		std::string name = App->fsystem->GetFilename(file.c_str());
		unsigned uid = std::stoul(name);
		ResourceMesh* res = (ResourceMesh*)CreateNewResource(TYPE::MESH, uid);	
		res->SetExportedFile(name.c_str());
	}
	files.clear();
	dirs.clear();
	return true;
}

void ModuleResourceManager::LoadEngineResources()
{
	std::vector<std::string> files;
	std::vector<std::string> dirs;
	App->fsystem->ListFolderContent(IMPORTED_RESOURCES, files, dirs);
	for each (std::string file in files)
	{
		Resource* res = CreateNewResource(TYPE::TEXTURE);
		res->SetExportedFile(App->fsystem->GetFilename(file.c_str()).c_str());
		std::string filePath(IMPORTED_RESOURCES);
		res->SetFile((filePath + file).c_str());
		res->SetUsedByEngine(true);
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

unsigned ModuleResourceManager::FindByFileInAssets(const char* fileInAssets) const
{
	for (std::map<unsigned, Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (strcmp(it->second->GetFile(), fileInAssets) == 0)
		{
			return it->first;
		}
	}
	return 0;
}

unsigned ModuleResourceManager::FindByExportedFile(const char* exportedFileName) const
{
	for (std::map<unsigned, Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (strcmp(it->second->GetExportedFile(), exportedFileName) == 0)
		{
			return it->first;
		}
	}
	return 0;
}

unsigned ModuleResourceManager::ImportFile(const char* newFileInAssets, const char* filePath, TYPE type, bool force)
{
	unsigned ret = 0; 
	bool success = false; 
	std::string importedFilePath(filePath);

	Resource* resource = CreateNewResource(type);
	std::string assetPath(filePath);
	assetPath += newFileInAssets;

	switch (type) 
	{
	case TYPE::TEXTURE: 
		success = App->textures->ImportImage(newFileInAssets, filePath, (ResourceTexture*)resource);
		break;
	case TYPE::MESH:	
		success = App->fsystem->importer.ImportFBX(newFileInAssets, filePath);
		break;
	//case TYPE::AUDIO: import_ok = App->audio->Import(newFileInAssets, written_file); break;
	//case TYPE::SCENE: import_ok = App->scene->Import(newFileInAssets, written_file); break;
	case TYPE::MATERIAL:
		success = App->fsystem->Copy(filePath, IMPORTED_MATERIALS, newFileInAssets);
		break;
	}

	// If export was successful, create a new resource
	if (success) 
	{ 
		resource->SaveMetafile(assetPath.c_str());
		resource->SetFile((importedFilePath + newFileInAssets).c_str());
		resource->SetExportedFile(App->fsystem->RemoveExtension(newFileInAssets).c_str());
	}
	else
	{
		RELEASE(resource);
	}
	return ret;
}

Resource * ModuleResourceManager::CreateNewResource(TYPE type, unsigned forceUid)
{
	Resource* resource = nullptr;
	unsigned uid = (forceUid == 0) ? GenerateNewUID() : forceUid;

	switch (type) 
	{
	case TYPE::TEXTURE: resource = (Resource*) new ResourceTexture(uid); break;
	case TYPE::MESH:	resource = (Resource*) new ResourceMesh(uid); break;
	/*case TYPE::AUDIO:	resource = (Resource*) new ResourceAudio(uid); break;
	case TYPE::SCENE:	resource = (Resource*) new ResourceScene(uid); break;
	case TYPE::ANIMATION: resource = (Resource*) new ResourceAnimation(uid); break;*/
	case TYPE::MATERIAL: resource = (Resource*) new ResourceMaterial(uid); break;
	case TYPE::SKYBOX: resource = (Resource*) new ResourceSkybox(uid); break;
	}

	if (resource != nullptr)
		resources[uid] = resource;

	return resource;
}

unsigned ModuleResourceManager::GenerateNewUID()
{
	return App->scene->GetNewUID();
}

Resource* ModuleResourceManager::Get(unsigned uid) const
{
	std::map<unsigned, Resource*>::const_iterator it = resources.find(uid);
	if (it == resources.end())
		return nullptr;

	Resource* resource = it->second;
	// Check if is already loaded in memory
	if (!resource->IsLoadedToMemory())
	{
		// Load in memory
		if (resource->LoadInMemory())
			return resource;
		else
			return nullptr;
	}
	else
	{
		resource->SetReferences(resource->GetReferences() + 1);
		return resource;
	}
	return nullptr;
}

Resource* ModuleResourceManager::Get(const char* exportedFileName) const
{
	assert(exportedFileName != NULL);

	// Look for it on the resource list
	unsigned uid = FindByExportedFile(exportedFileName);
	if (uid == 0)
		return nullptr;

	// Get resource by uid
	return Get(uid);
}

Resource* ModuleResourceManager::GetWithoutLoad(unsigned uid) const
{
	std::map<unsigned, Resource*>::const_iterator it = resources.find(uid);
	if (it == resources.end())
		return nullptr;

	return it->second;
}

Resource* ModuleResourceManager::GetWithoutLoad(const char* exportedFileName) const
{
	assert(exportedFileName != NULL);

	// Look for it on the resource list
	unsigned uid = FindByExportedFile(exportedFileName);
	if (uid == 0)
		return nullptr;

	// Get resource by uid
	return GetWithoutLoad(uid);
}

bool ModuleResourceManager::DeleteResource(unsigned uid)
{
	std::map<unsigned, Resource*>::iterator it = resources.find(uid);
	if (it != resources.end())
	{
		if (it->second->GetReferences() > 1)
		{
			it->second->SetReferences(it->second->GetReferences() - 1);
			return true;
		}
		else if(it->second->IsLoadedToMemory())
		{
			it->second->DeleteFromMemory();
			return true;
		}
	}
	return false;
}

std::vector<Resource*> ModuleResourceManager::GetResourcesList()
{
	std::vector<Resource*> resourcesList;
	for (std::map<unsigned, Resource*>::iterator it = resources.begin(); it != resources.end(); ++it)
	{
		resourcesList.push_back(it->second);
	}
	return resourcesList;
}

Resource* ModuleResourceManager::AddResource(const char* file, const char* directory, TYPE type)
{
	std::string path(directory);
	path += file;
	// Check if resource was already added
	unsigned UID = FindByFileInAssets(path.c_str());
	if ( UID == 0)
	{
		// Create new resource 
		Resource* res = CreateNewResource(type);
		res->SetExportedFile(App->fsystem->GetFilename(file).c_str());
		res->SetFile((path).c_str());
		return res;
	}
	else
	{
		// Resource already exist
		return GetWithoutLoad(UID);
	}
}

void ModuleResourceManager::DeleteResourceFromList(unsigned uid)
{
	std::map<unsigned, Resource*>::const_iterator it = resources.find(uid);
	if (it != resources.end())
		resources.erase(it);
}
