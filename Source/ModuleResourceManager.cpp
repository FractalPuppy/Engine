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
#include "ResourceModel.h"
#include "ResourceMesh.h"
#include "ResourceMaterial.h"
#include "ResourceStateMachine.h"
#include "ResourceAnimation.h"
#include "ResourceSkybox.h"

#include "FileImporter.h"

#include <algorithm>

bool sortByNameAscending(const std::string a, std::string b) { return a < b; };

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

void ModuleResourceManager::LoadEngineResources()
{
	std::set<std::string> files;
	App->fsystem->ListFiles(IMPORTED_RESOURCES, files);
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

unsigned ModuleResourceManager::FindByFileInAssetsExcludingType(const char* fileInAssets, TYPE type) const
{
	for (std::map<unsigned, Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (strcmp(it->second->GetFile(), fileInAssets) == 0 && it->second->GetType() != type)
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

unsigned ModuleResourceManager::FindByExportedFile(const char* exportedFileName, TYPE type) const
{
	for (std::map<unsigned, Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (strcmp(it->second->GetExportedFile(), exportedFileName) == 0 && it->second->GetType() == type)
		{
			return it->first;
		}
	}
	return 0;
}

bool ModuleResourceManager::ImportFile(const char* newFileInAssets, const char* filePath, TYPE type)
{
	bool success = false; 

	std::string assetPath(filePath);
	assetPath += newFileInAssets;

	// Check if the file was already imported (Mesh is excluded because has same file as model)
	unsigned meshUID = FindByFileInAssetsExcludingType(assetPath.c_str(), TYPE::MESH);

	if (meshUID != 0)
	{
		unsigned animUID = FindByFileInAssetsExcludingType(assetPath.c_str(), TYPE::ANIMATION);
		// Avoid reimporting meshes (only model can reimport them)
		if (animUID != 0)
		{
			return ReImportFile(GetWithoutLoad(animUID), filePath, type);
		}
	}



	Resource* resource = CreateNewResource(type);

	// Save file to import on Resource file variable
	resource->SetFile((assetPath).c_str());

	// If a .meta file is found import it with the configuration saved
	resource->LoadConfigFromMeta();

	switch (type) 
	{
	case TYPE::TEXTURE: 
		success = App->textures->ImportImage(newFileInAssets, filePath, (ResourceTexture*)resource);
		break;
	case TYPE::MODEL:
		success = App->fsystem->importer.ImportFBX(newFileInAssets, filePath, (ResourceModel*)resource);
		break;
	/*case TYPE::MESH:	
		success = App->fsystem->importer.ImportFBX(newFileInAssets, filePath, (ResourceMesh*)resource);
		break;*/
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
		resource->SetExportedFile(App->fsystem->RemoveExtension(newFileInAssets).c_str());
		LOG("%s imported.", resource->GetExportedFile());
	}
	else
	{
		RELEASE(resource);
	}
	return success;
}

bool ModuleResourceManager::ReImportFile(Resource* resource, const char* filePath, TYPE type)
{
	bool success = false;

	std::string file = resource->GetExportedFile();
	file += App->fsystem->GetExtension(resource->GetFile());

	switch (type)
	{
	case TYPE::TEXTURE:
		success = App->textures->ImportImage(file.c_str(), filePath, (ResourceTexture*)resource);
		break;
	case TYPE::MODEL:
		success = App->fsystem->importer.ImportFBX(file.c_str(), filePath, (ResourceModel*)resource);
		break;
	/*case TYPE::MESH:
		success = App->fsystem->importer.ImportFBX(file.c_str(), filePath, (ResourceMesh*)resource);
		break;*/
		//case TYPE::AUDIO: import_ok = App->audio->Import(newFileInAssets, written_file); break;
		//case TYPE::SCENE: import_ok = App->scene->Import(newFileInAssets, written_file); break;
	case TYPE::MATERIAL:
		success = App->fsystem->Copy(filePath, IMPORTED_MATERIALS, file.c_str());
		break;
	}

	// If export was successful, update resource
	if (success)
	{
		std::string assetPath(filePath);
		assetPath += file;
		resource->SaveMetafile(assetPath.c_str());

		// Reload in memory
		if (resource->IsLoadedToMemory())
		{
			unsigned references = resource->GetReferences();
			resource->DeleteFromMemory();
			resource->SetReferences(references);
		}

		LOG("%s reimported.", resource->GetExportedFile());
	}
	else
	{
		LOG("Error: %s failed on reimport.", resource->GetExportedFile());
	}
	return success;
}

Resource* ModuleResourceManager::CreateNewResource(TYPE type, unsigned forceUid)
{
	Resource* resource = nullptr;
	unsigned uid = (forceUid == 0) ? GenerateNewUID() : forceUid;

	switch (type) 
	{
	case TYPE::TEXTURE: resource = (Resource*) new ResourceTexture(uid); break;
	case TYPE::MODEL:	resource = (Resource*) new ResourceModel(uid); break;
	case TYPE::MESH:	resource = (Resource*) new ResourceMesh(uid); break;
	/*case TYPE::AUDIO:	resource = (Resource*) new ResourceAudio(uid); break;
	case TYPE::SCENE:	resource = (Resource*) new ResourceScene(uid); break;*/
	case TYPE::ANIMATION: resource = (Resource*) new ResourceAnimation(uid); break;
	case TYPE::MATERIAL: resource = (Resource*) new ResourceMaterial(uid); break;
	case TYPE::SKYBOX: resource = (Resource*) new ResourceSkybox(uid); break;
	case TYPE::STATEMACHINE: resource = (Resource*) new ResourceStateMachine(uid); break;
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

Resource* ModuleResourceManager::Get(const char* exportedFileName, TYPE type) const
{
	assert(exportedFileName != NULL);

	// Look for it on the resource list
	unsigned uid = FindByExportedFile(exportedFileName, type);
	if (uid == 0u)
		return nullptr;

	// Get resource by uid
	return Get(uid);
}

ResourceMesh* ModuleResourceManager::GetMeshByName(const char* name)
{
	std::vector<std::string> resourcesList;
	for (std::map<unsigned, Resource*>::iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (it->second->GetType() == TYPE::MESH)
		{
			ResourceMesh* mesh = (ResourceMesh*)it->second;
			if (mesh->name == name)
				return (ResourceMesh*)Get(mesh->GetUID());
		}
	}

	return nullptr;
}

ResourceAnimation* ModuleResourceManager::GetAnimationByName(const char* name)
{
	std::vector<std::string> resourcesList;
	for (std::map<unsigned, Resource*>::iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (it->second->GetType() == TYPE::ANIMATION)
		{
			ResourceAnimation* anim = (ResourceAnimation*)it->second;
			if (anim->name == name)
				return (ResourceAnimation*)Get(anim->GetUID());
		}
	}
	return nullptr;
}

ResourceStateMachine* ModuleResourceManager::GetSMByName(const char * name)
{
	std::vector<std::string> resourcesList;
	for (std::map<unsigned, Resource*>::iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (it->second->GetType() == TYPE::STATEMACHINE)
		{
			ResourceStateMachine* stateMachine = (ResourceStateMachine*)it->second;
			if (stateMachine->name == name)
				return (ResourceStateMachine*)Get(stateMachine->GetUID());
		}
	}
	return nullptr;
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

Resource* ModuleResourceManager::GetWithoutLoad(const char* exportedFileName, TYPE type) const
{
	assert(exportedFileName != NULL);

	// Look for it on the resource list
	unsigned uid = FindByExportedFile(exportedFileName,type);
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

std::vector<ResourceTexture*> ModuleResourceManager::GetTexturesList()
{
	std::vector<ResourceTexture*> resourcesList;
	for (std::map<unsigned, Resource*>::iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if(it->second->GetType() == TYPE::TEXTURE)
			resourcesList.push_back((ResourceTexture*)it->second);
	}
	return resourcesList;
}

std::vector<ResourceMaterial*> ModuleResourceManager::GetMaterialsList()
{
	std::vector<ResourceMaterial*> resourcesList;
	for (std::map<unsigned, Resource*>::iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (it->second->GetType() == TYPE::MATERIAL)
			resourcesList.push_back((ResourceMaterial*)it->second);
	}
	return resourcesList;
}

std::vector<ResourceAnimation*> ModuleResourceManager::GetAnimationsList()
{
	std::vector<ResourceAnimation*> resourcesList;
	for (std::map<unsigned, Resource*>::iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (it->second->GetType() == TYPE::ANIMATION)
			resourcesList.push_back((ResourceAnimation*)it->second);
	}
	return resourcesList;
}

std::vector<ResourceStateMachine*> ModuleResourceManager::GetSMList()
{
	std::vector<ResourceStateMachine*> resourcesList;
	for (std::map<unsigned, Resource*>::iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (it->second->GetType() == TYPE::STATEMACHINE)
			resourcesList.push_back((ResourceStateMachine*)it->second);
	}
	return resourcesList;
}

std::vector<std::string> ModuleResourceManager::GetResourceNamesList(TYPE resourceType, bool ordered)
{
	std::vector<std::string> resourcesList;
	for (std::map<unsigned, Resource*>::iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (it->second->GetType() == resourceType)
			resourcesList.push_back(it->second->GetExportedFile());
	}

	if (ordered)	// Short by ascending order
		std::sort(resourcesList.begin(), resourcesList.end(), sortByNameAscending);

	return resourcesList;
}

std::vector<std::string> ModuleResourceManager::GetMeshesNamesList(bool ordered)
{
	std::vector<std::string> resourcesList;
	for (std::map<unsigned, Resource*>::iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (it->second->GetType() == TYPE::MESH)
		{
			ResourceMesh* mesh = (ResourceMesh*)it->second;
			resourcesList.push_back(mesh->name);
		}
	}

	if (ordered)	// Short by ascending order
		std::sort(resourcesList.begin(), resourcesList.end(), sortByNameAscending);

	return resourcesList;
}

std::vector<std::string> ModuleResourceManager::GetAnimationsNamesList(bool ordered)
{
	std::vector<std::string> resourcesList;
	for (std::map<unsigned, Resource*>::iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (it->second->GetType() == TYPE::ANIMATION)
		{
			ResourceAnimation* anim = (ResourceAnimation*)it->second;
			resourcesList.push_back(anim->name);
		}
	}

	if (ordered)	// Short by ascending order
		std::sort(resourcesList.begin(), resourcesList.end(), sortByNameAscending);

	return resourcesList;
}

std::vector<std::string> ModuleResourceManager::GetSMNamesList(bool ordered)
{
	std::vector<std::string> resourcesList;
	for (std::map<unsigned, Resource*>::iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (it->second->GetType() == TYPE::STATEMACHINE)
		{
			ResourceStateMachine* stateMachine = (ResourceStateMachine*)it->second;
			resourcesList.push_back(stateMachine->name);
		}
	}

	if (ordered)	// Short by ascending order
		std::sort(resourcesList.begin(), resourcesList.end(), sortByNameAscending);

	return resourcesList;
}

bool ModuleResourceManager::Exists(const char* exportedFileName)
{
	for (std::map<unsigned, Resource*>::iterator it = resources.begin(); it != resources.end(); ++it)
	{
		std::string firstName(it->second->GetExportedFile());
		std::string secondName(exportedFileName);
		if (firstName == secondName)
			return true;
		else
		{
			for (auto & c1 : firstName) c1 = toupper(c1);
			for (auto & c2 : secondName) c2 = toupper(c2);
			if (firstName == secondName)
				return true;
		}
	}
	return false;
}

bool ModuleResourceManager::Exists(const char* exportedFileName, TYPE type)
{
	for (std::map<unsigned, Resource*>::iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (it->second->GetType() == type)
		{
			std::string firstName(it->second->GetExportedFile());
			std::string secondName(exportedFileName);
			if (firstName == secondName)
				return true;
			else
			{
				for (auto & c1 : firstName) c1 = toupper(c1);
				for (auto & c2 : secondName) c2 = toupper(c2);
				if (firstName == secondName)
					return true;
			}
		}
	}
	return false;
}


Resource* ModuleResourceManager::AddResource(const char* file, const char* directory, TYPE type)
{
	std::string path(directory);
	path += file;

	if (type == TYPE::STATEMACHINE)
	{
		// Create new resource 
		//unsigned smUID = App->fsystem->RemoveExtension(file)
		Resource* res = CreateNewResource(type , std::stoul(App->fsystem->RemoveExtension(file), nullptr, 0));
		res->LoadInMemory();
		res->SetExportedFile(App->fsystem->GetFilename(file).c_str());
		res->SetFile((path).c_str());
		return res;
	}

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

Resource* ModuleResourceManager::ReplaceResource(unsigned oldResourceUID, Resource* newResource)
{
	DeleteResourceFromList(oldResourceUID);
	resources[newResource->GetUID()] = newResource;

	return nullptr;
}

void ModuleResourceManager::DeleteResourceFromList(unsigned uid)
{
	std::map<unsigned, Resource*>::const_iterator it = resources.find(uid);
	if (it != resources.end())
		resources.erase(it);
}