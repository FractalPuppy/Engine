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
#include "ResourcePrefab.h"
#include "ResourceScene.h"
#include "ResourceAudio.h"

#include "FileImporter.h"

#include "JSON.h"
#include "HashString.h"
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

bool ModuleResourceManager::CleanUp()
{
	// Delete every resource from memory
	for each (auto& resource in resources)
	{
		if (resource.second->IsLoadedToMemory())
			resource.second->DeleteFromMemory();
	}

	// Delete pointers
	for (auto it = resources.begin(); it != resources.end(); it++)
	{
		RELEASE(it->second);
	}
	resources.clear();
	return true;
}

void ModuleResourceManager::LoadEngineResources()
{
	std::set<std::string> files;
	App->fsystem->ListFileNames(IMPORTED_RESOURCES, files);
	for each (std::string file in files)
	{
		Resource* res = CreateNewResource(TYPE::TEXTURE);
		res->SetName(App->fsystem->GetFilename(file).c_str());
		res->SetExportedFile((IMPORTED_RESOURCES + file + TEXTUREEXT).c_str());
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

unsigned ModuleResourceManager::FindByFileInAssetsOfType(const char * fileInAssets, TYPE type) const
{
	for (std::map<unsigned, Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (strcmp(it->second->GetFile(), fileInAssets) == 0 && it->second->GetType() == type)
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

unsigned ModuleResourceManager::FindByExportedFile(const char* exportedFile) const
{
	for (std::map<unsigned, Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
	{
		std::string firstName(it->second->GetExportedFile());
		std::string secondName(exportedFile);
		if (firstName == secondName)
			return it->first;
		else
		{
			for (auto & c1 : firstName) c1 = toupper(c1);
			for (auto & c2 : secondName) c2 = toupper(c2);
			if (firstName == secondName)
				return it->first;
		}
	}
	return 0;
}

unsigned ModuleResourceManager::FindByExportedFile(const char* exportedFile, TYPE type) const
{
	for (std::map<unsigned, Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (it->second->GetType() == type)
		{
			std::string firstName(it->second->GetExportedFile());
			std::string secondName(exportedFile);
			if (firstName == secondName)
				return it->first;
			else
			{
				for (auto & c1 : firstName) c1 = toupper(c1);
				for (auto & c2 : secondName) c2 = toupper(c2);
				if (firstName == secondName)
					return it->first;
			}
		}
	}
	return 0;
}

unsigned ModuleResourceManager::FindByName(const char* name, TYPE type) const
{
	for (std::map<unsigned, Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (HashString(it->second->GetName()) == HashString(name) && it->second->GetType() == type)
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

	// Check if the file was already imported
	unsigned resUID = FindByFileInAssetsOfType(assetPath.c_str(), type);
	if (resUID != 0)
	{
		return ReImportFile(GetWithoutLoad(resUID), filePath, type);
	}

	Resource* resource = CreateNewResource(type);

	// Save file to import on Resource file variable
	resource->SetFile(assetPath.c_str());

	// If a .meta file is found import it with the configuration saved
	resource->LoadConfigFromMeta();

	// Import resource and set exported file
	std::string exportedFile;
	std::string name = App->fsystem->RemoveExtension(newFileInAssets);

	switch (type) 
	{
	case TYPE::TEXTURE:		
		success = App->textures->ImportImage(newFileInAssets, filePath,	(ResourceTexture*)resource);
		exportedFile = TEXTURES + std::to_string(resource->GetUID()) + TEXTUREEXT;
		break;
	case TYPE::MODEL:
		success = App->fsystem->importer.ImportFBX(newFileInAssets, filePath, (ResourceModel*)resource);
		exportedFile = name + FBXEXTENSION;
		break;
	/*case TYPE::MESH:		
		success = App->fsystem->importer.ImportFBX(newFileInAssets, filePath, (ResourceMesh*)resource);	
		exportedFile = MESHES + name + MESHEXTENSION;	
		break;*/
	case TYPE::AUDIO:
		// TODO: Add importer for audios
		//success = App->audio->Import(newFileInAssets, written_file);	
		success = App->fsystem->Copy(filePath, IMPORTED_AUDIOS, newFileInAssets);
		if (success)
		{
			success = App->fsystem->Rename(IMPORTED_AUDIOS, (name + App->fsystem->GetExtension(newFileInAssets)).c_str(), std::to_string(resource->GetUID()).c_str());
		}
		exportedFile = IMPORTED_AUDIOS + std::to_string(resource->GetUID()) + App->fsystem->GetExtension(newFileInAssets);
		break;
	case TYPE::SCENE:		
		success = App->fsystem->Copy(filePath, IMPORTED_SCENES, newFileInAssets);	
		if (success)
		{
			success = App->fsystem->Rename(IMPORTED_SCENES, (name + SCENEEXTENSION).c_str(), std::to_string(resource->GetUID()).c_str());
		}
		exportedFile = IMPORTED_SCENES + std::to_string(resource->GetUID()) + SCENEEXTENSION;
		break;
	case TYPE::MATERIAL:	
		success = App->fsystem->Copy(filePath, IMPORTED_MATERIALS, newFileInAssets);	
		if (success)
		{
			success = App->fsystem->Rename(IMPORTED_MATERIALS, (name + MATERIALEXT).c_str(), std::to_string(resource->GetUID()).c_str());
		}
		exportedFile = IMPORTED_MATERIALS + std::to_string(resource->GetUID()) + MATERIALEXT;
		break;
	case TYPE::ANIMATION:
		success = App->fsystem->Copy(filePath, IMPORTED_ANIMATIONS, newFileInAssets);
		if (success)
		{
			success = App->fsystem->Rename(IMPORTED_ANIMATIONS, (name + ANIMATIONEXTENSION).c_str(), std::to_string(resource->GetUID()).c_str());
		}
		exportedFile = IMPORTED_ANIMATIONS + std::to_string(resource->GetUID()) + ANIMATIONEXTENSION;
		break;
	case TYPE::STATEMACHINE:
		success = App->fsystem->Copy(filePath, IMPORTED_STATEMACHINES, newFileInAssets);
		if (success)
		{
			success = App->fsystem->Rename(IMPORTED_STATEMACHINES, (name + STATEMACHINEEXTENSION).c_str(), std::to_string(resource->GetUID()).c_str());
		}
		exportedFile = IMPORTED_STATEMACHINES + std::to_string(resource->GetUID()) + STATEMACHINEEXTENSION;
		break;
	case TYPE::PREFAB: //TODO: update reimport PREFAB
		success = App->fsystem->importer.ImportPrefab(newFileInAssets, filePath, (ResourcePrefab*)resource);
		exportedFile = IMPORTED_PREFABS + std::to_string(resource->GetUID()) + PREFABEXTENSION;
		break;
	}

	// If export was successful, create a new resource
	if (success) 
	{ 
		resource->SaveMetafile(assetPath.c_str());
		resource->SetName(name.c_str());
		resource->SetExportedFile(exportedFile.c_str());
		LOG("%s imported.", resource->GetFile());
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

	std::string file = App->fsystem->GetFile(resource->GetFile());

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
	case TYPE::AUDIO:	
	// TODO: Add importer for audios
	//success = App->audio->Import(newFileInAssets, written_file);	
		success = App->fsystem->Copy(filePath, IMPORTED_AUDIOS, file.c_str());
		if (success)
		{
			success = App->fsystem->Rename(IMPORTED_AUDIOS, App->fsystem->GetFile(file).c_str(), std::to_string(resource->GetUID()).c_str());
		}
	break;
	case TYPE::MATERIAL:	
		success = App->fsystem->Copy(filePath, IMPORTED_MATERIALS, file.c_str());
		if (success)
		{
			success = App->fsystem->Rename(IMPORTED_MATERIALS, App->fsystem->GetFile(file).c_str(), std::to_string(resource->GetUID()).c_str());
		}
		break;
	case TYPE::SCENE:	
		success = App->fsystem->Copy(filePath, IMPORTED_SCENES, file.c_str());			
		if (success)
		{
			success = App->fsystem->Rename(IMPORTED_SCENES, App->fsystem->GetFile(file).c_str(), std::to_string(resource->GetUID()).c_str());
		}
		break;
	case TYPE::ANIMATION:	
		success = App->fsystem->Copy(filePath, IMPORTED_ANIMATIONS, file.c_str());		
		if (success)
		{
			success = App->fsystem->Rename(IMPORTED_ANIMATIONS, App->fsystem->GetFile(file).c_str(),
				std::to_string(resource->GetUID()).c_str());
		}
		break;
	case TYPE::STATEMACHINE:
		success = App->fsystem->Copy(filePath, IMPORTED_STATEMACHINES, file.c_str());	
		if (success)
		{
			success = App->fsystem->Rename(IMPORTED_STATEMACHINES, App->fsystem->GetFile(file).c_str(),
				std::to_string(resource->GetUID()).c_str());
		}
		break;
	case TYPE::PREFAB:
		success = App->fsystem->importer.ImportPrefab(file.c_str(), filePath, (ResourcePrefab*)resource);
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

		LOG("%s reimported.", resource->GetFile());
	}
	else
	{
		LOG("Error: %s failed on reimport.", resource->GetFile());
	}
	return success;
}

Resource* ModuleResourceManager::CreateNewResource(TYPE type, unsigned forceUid)
{
	Resource* resource = nullptr;
	unsigned uid = (forceUid == 0) ? GenerateNewUID() : forceUid;

	switch (type) 
	{
	case TYPE::TEXTURE:			resource = (Resource*) new ResourceTexture(uid);		break;
	case TYPE::MODEL:			resource = (Resource*) new ResourceModel(uid);			break;
	case TYPE::MESH:			resource = (Resource*) new ResourceMesh(uid);			break;
	case TYPE::SCENE:			resource = (Resource*) new ResourceScene(uid);			break;
	case TYPE::ANIMATION:		resource = (Resource*) new ResourceAnimation(uid);		break;
	case TYPE::MATERIAL:		resource = (Resource*) new ResourceMaterial(uid);		break;
	case TYPE::SKYBOX:			resource = (Resource*) new ResourceSkybox(uid);			break;
	case TYPE::STATEMACHINE:	resource = (Resource*) new ResourceStateMachine(uid);	break;
	case TYPE::AUDIO:			resource = (Resource*) new ResourceAudio(uid);			break;
	case TYPE::PREFAB:          resource = (Resource*) new ResourcePrefab(uid);			break;
	/*case TYPE::AUDIO:			resource = (Resource*) new ResourceAudio(uid);			break;*/
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
	if (uid == 0u) return nullptr;

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

Resource* ModuleResourceManager::Get(const char* exportedFile) const
{
	assert(exportedFile != NULL);

	// Look for it on the resource list
	unsigned uid = FindByExportedFile(exportedFile);
	if (uid == 0)
		return nullptr;

	// Get resource by uid
	return Get(uid);
}

Resource* ModuleResourceManager::Get(const char* exportedFile, TYPE type) const
{
	assert(exportedFile != NULL);

	// Look for it on the resource list
	unsigned uid = FindByExportedFile(exportedFile, type);
	if (uid == 0u)
		return nullptr;

	// Get resource by uid
	return Get(uid);
}

Resource* ModuleResourceManager::GetByName(const char* name, TYPE type)
{
	std::vector<std::string> resourcesList;
	for (std::map<unsigned, Resource*>::iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (it->second->GetType() == type)
		{
			Resource* res = it->second;
			if (HashString(res->GetName()) == HashString(name))
			{
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
			}
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

Resource* ModuleResourceManager::GetWithoutLoad(const char* exportedFile) const
{
	assert(exportedFile != NULL);

	// Look for it on the resource list
	unsigned uid = FindByExportedFile(exportedFile);
	if (uid == 0)
		return nullptr;

	// Get resource by uid
	return GetWithoutLoad(uid);
}

Resource* ModuleResourceManager::GetWithoutLoad(const char* exportedFile, TYPE type) const
{
	assert(exportedFile != NULL);

	// Look for it on the resource list
	unsigned uid = FindByExportedFile(exportedFile,type);
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
			resourcesList.push_back(it->second->GetName());
	}

	if (ordered)	// Short by ascending order
		std::sort(resourcesList.begin(), resourcesList.end(), sortByNameAscending);

	return resourcesList;
}

TYPE ModuleResourceManager::GetResourceType(FILETYPE fileType) const
{
	switch (fileType)
	{
	case FILETYPE::TEXTURE:				return TYPE::TEXTURE;		break;
	case FILETYPE::IMPORTED_TEXTURE:	return TYPE::TEXTURE;		break;
	case FILETYPE::MODEL:				return TYPE::MODEL;			break;
	case FILETYPE::IMPORTED_MESH: 		return TYPE::MESH;			break;
	case FILETYPE::SCENE:				return TYPE::SCENE;			break;
	case FILETYPE::ANIMATION:			return TYPE::ANIMATION;		break;
	case FILETYPE::MATERIAL:			return TYPE::MATERIAL;		break;
	case FILETYPE::SKYBOX:				return TYPE::SKYBOX;		break;
	case FILETYPE::STATEMACHINE:		return TYPE::STATEMACHINE;	break;
	case FILETYPE::AUDIO:				return TYPE::AUDIO;			break;
	case FILETYPE::PREFAB:				return TYPE::PREFAB;		break;
	default:
	case FILETYPE::NONE:				return TYPE::UNKNOWN;		break;
	}
	return TYPE::UNKNOWN;
}

bool ModuleResourceManager::Exists(const char* exportedFile)
{
	for (std::map<unsigned, Resource*>::iterator it = resources.begin(); it != resources.end(); ++it)
	{
		std::string firstName(it->second->GetExportedFile());
		std::string secondName(exportedFile);
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

bool ModuleResourceManager::Exists(const char* exportedFile, TYPE type)
{
	for (std::map<unsigned, Resource*>::iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (it->second->GetType() == type)
		{
			std::string firstName(it->second->GetExportedFile());
			std::string secondName(exportedFile);
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

bool ModuleResourceManager::NameExists(const char* name, TYPE type)
{
	for (std::map<unsigned, Resource*>::iterator it = resources.begin(); it != resources.end(); ++it)
	{
		if (it->second->GetType() == type)
		{
			std::string firstName(it->second->GetName());
			std::string secondName(name);
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

std::string ModuleResourceManager::GetAvailableName(const char* name, TYPE type)
{
	std::string newName(name);
	unsigned i = 1u;
	while (NameExists(newName.c_str(), type))
	{
		newName = name + std::to_string(i);
		i++;

		// Emergency control
		if (i > 999u)
		{
			newName = App->resManager->GenerateNewUID();
			break;
		}
	}
	return newName;
}


Resource* ModuleResourceManager::AddResource(const char* file, const char* directory, TYPE type)
{
	std::string path(directory);
	path += file;
	std::string exportedFile;
	std::string name(App->fsystem->GetFilename(file));

	// Check if resource was already added
	unsigned UID = FindByFileInAssets(path.c_str());
	if ( UID == 0)
	{
		// Create new resource 
		Resource* res = CreateNewResource(type);
		res->SetName(name.c_str());
		res->SetFile(path.c_str());
		return res;
	}
	else
	{
		// Resource already exist
		return GetWithoutLoad(UID);
	}
}

Resource* ModuleResourceManager::AddResource(const char* file, const char* directory, TYPE type, unsigned uid)
{
	std::string path(directory);
	path += file;
	std::string exportedFile;
	std::string name(App->fsystem->GetFilename(file));
 
	// Check if resource was already added
	if (uid != FindByFileInAssets(path.c_str()))
	{
		// Create new resource 
		Resource* res = CreateNewResource(type, uid);

		// Set Exported File
		switch (type)
		{
		case TYPE::TEXTURE:		exportedFile = TEXTURES					+ std::to_string(uid) + TEXTUREEXT;				break;
		case TYPE::MODEL:		exportedFile = App->fsystem->GetFilename(file)				  + FBXEXTENSION;			break;
		/*case TYPE::MESH:		exportedFile = MESHES;					+ std::to_string(uid) + MESHEXTENSION;			break;*/
		case TYPE::AUDIO:		exportedFile = IMPORTED_AUDIOS			+ std::to_string(uid) + App->fsystem->GetExtension(file);			break;
		case TYPE::SCENE:		exportedFile = IMPORTED_SCENES			+ std::to_string(uid) + SCENEEXTENSION;			break;
		case TYPE::MATERIAL:	exportedFile = IMPORTED_MATERIALS		+ std::to_string(uid) + MATERIALEXT;			break;
		case TYPE::ANIMATION:	exportedFile = IMPORTED_ANIMATIONS		+ std::to_string(uid) + ANIMATIONEXTENSION;		break;
		case TYPE::STATEMACHINE:exportedFile = IMPORTED_STATEMACHINES	+ std::to_string(uid) + STATEMACHINEEXTENSION;	break;
		default:
			break;
			}
		res->SetName(name.c_str());
		res->SetExportedFile(exportedFile.c_str());
		res->SetFile(path.c_str());
		return res;
	}
	else
	{
		// Resource already exist
		return GetWithoutLoad(uid);
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

unsigned ModuleResourceManager::GetUIDFromMeta(const char* metaFile, FILETYPE fileType) const
{
	char* data = nullptr;
	if (App->fsystem->Load(metaFile, &data) == 0)
	{
		LOG("Warning: %s couldn't be loaded", metaFile);
		RELEASE_ARRAY(data);
		return 0;
	}

	JSON* json = new JSON(data);
	JSON_value* value = nullptr;
	TYPE type = GetResourceType(fileType);
	switch (type)
	{
	case TYPE::TEXTURE:		value = json->GetValue("Texture");		break;
	case TYPE::MATERIAL:	value = json->GetValue("Material");		break;
	case TYPE::MESH:		value = json->GetValue("Mesh");			break;
	case TYPE::AUDIO:		value = json->GetValue("Audio");		break;
	case TYPE::MODEL:		value = json->GetValue("Mesh");			break;
	case TYPE::SCENE:		value = json->GetValue("Scene");		break;
	case TYPE::ANIMATION:	value = json->GetValue("Animation");	break;
	case TYPE::STATEMACHINE:value = json->GetValue("StateMachine");	break;
	case TYPE::PREFAB:      value = json->GetValue("Prefab");		break;
	default:
		return 0;
		break;
	}
	if (value != nullptr)
	{
		return value->GetUint("GUID");
	}
	else
	{
		return 0;
	}
}

void ModuleResourceManager::CleanUnusedMetaFiles() const
{
	// Get lists with all assets
	std::set<std::string> assetFiles;
	App->fsystem->ListFilesWithExtension(ASSETS, assetFiles);

	for (auto& file : assetFiles)
	{
		if (HashString(App->fsystem->GetExtension(file).c_str()) == HashString(METAEXT))
		{
			std::string fileAssignedToMeta = App->fsystem->RemoveExtension(file);
			if (!App->fsystem->Exists(fileAssignedToMeta.c_str()))
			{
				App->fsystem->Delete(file.c_str());
			}
		}
	}
}

void ModuleResourceManager::CleanUnusedExportedFiles() const
{
	// Get lists with all imported files
	std::set<std::string> importedFiles;
	App->fsystem->ListFilesWithExtension(LIBRARY, importedFiles);

	for (auto& file : importedFiles)
	{
		if (!App->resManager->Exists(file.c_str()))
		{
			App->fsystem->Delete(file.c_str());
		}
	}
}
