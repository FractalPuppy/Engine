#include "ResourceModel.h"

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h"

#include "ResourceMesh.h"
#include "ResourceScene.h"
#include "ResourceAnimation.h"

#include "JSON.h"
#include "imgui.h"

ResourceModel::ResourceModel(unsigned uid) : Resource(uid, TYPE::MODEL)
{
}

ResourceModel::ResourceModel(const ResourceModel& resource) : Resource(resource)
{
	numMeshes = resource.numMeshes;
	meshList = resource.meshList;
	numAnimations = resource.numAnimations;
	animationList = resource.animationList;
}


ResourceModel::~ResourceModel()
{
	//DeleteFromMemory();
}

bool ResourceModel::LoadInMemory()
{
	for each(auto mesh in meshList)
	{
		mesh->LoadInMemory();
	}
	for each(auto anim in animationList)
	{
		anim->LoadInMemory();
	}
	return true;
}

void ResourceModel::DeleteFromMemory()
{
	Resource::DeleteFromMemory();

	for each(auto mesh in meshList)
	{
		mesh->DeleteFromMemory();
	}
	for each(auto anim in animationList)
	{
		anim->DeleteFromMemory();
	}
}

void ResourceModel::SaveMetafile(const char* file) const
{
	std::string filepath;
	filepath.append(file);
	JSON *json = new JSON();

	JSON_value* meshMeta = json->CreateValue();
	struct stat statFile;
	stat(filepath.c_str(), &statFile);
	meshMeta->AddUint("GUID", UID);
	meshMeta->AddUint("timeCreated", statFile.st_ctime);
	meshMeta->AddUint("NumMeshes", numMeshes);
	for (int i = 0; i < numMeshes; ++i)
	{
		meshMeta->AddUint(("Mesh" + std::to_string(i)).c_str(), meshList[i]->GetUID());
	}
	json->AddValue("Mesh", *meshMeta);

	JSON_value* animMeta = json->CreateValue();
	struct stat statFileAnim;
	stat(filepath.c_str(), &statFileAnim);
	animMeta->AddUint("timeCreated", statFileAnim.st_ctime);
	animMeta->AddUint("NumAnimations", numAnimations);
	for (int i = 0; i < numAnimations; ++i)
	{
		animMeta->AddUint(("Animation" + std::to_string(i)).c_str(), animationList[i]->GetUID());
	}
	json->AddValue("Animation", *animMeta);


	filepath += METAEXT;
	App->fsystem->Save(filepath.c_str(), json->ToString().c_str(), json->Size());
	RELEASE(json);
}

void ResourceModel::LoadConfigFromMeta()
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
	JSON_value* value = json->GetValue("Mesh");
	numMeshes = value->GetUint("NumMeshes");
	std::string name = App->fsystem->GetFilename(file);

	// Make sure the UID from meta is the same
	unsigned checkUID = value->GetUint("GUID");
	if (oldUID != checkUID)
	{
		UID = checkUID;
		// Update resource UID on resource list
		App->resManager->ReplaceResource(oldUID, this);
	}

	for (int i = 0; i < numMeshes; ++i)
	{
		unsigned meshUID = value->GetUint(("Mesh" + std::to_string(i)).c_str());
		ResourceMesh* mesh = (ResourceMesh*)App->resManager->CreateNewResource(TYPE::MESH, meshUID);
		mesh->SetFile(file.c_str());

		// Set Exported File
		mesh->SetExportedFile((MESHES + std::to_string(mesh->GetUID()) + MESHEXTENSION).c_str());
		mesh->SetName((name + "_" + std::to_string(i)).c_str());

		meshList.push_back(mesh);
	}

	value = json->GetValue("Animation");
	numAnimations = value->GetUint("NumAnimations");

	for (int i = 0; i < numAnimations; ++i)
	{
		unsigned animUID = value->GetUint(("Animation" + std::to_string(i)).c_str());
		ResourceAnimation* anim = (ResourceAnimation*)App->resManager->CreateNewResource(TYPE::ANIMATION, animUID);
		anim->SetFile(file.c_str());
		anim->SetExportedFile((IMPORTED_ANIMATIONS + std::to_string(anim->GetUID()) + ANIMATIONEXTENSION).c_str());
		anim->SetName((name + "_" + std::to_string(i)).c_str());

		animationList.push_back(anim);
	}
	RELEASE(json);
}

bool ResourceModel::CheckImportedMeshes()
{
	std::set<std::string> importedMeshes;
	App->fsystem->ListFilesWithExtension(MESHES, importedMeshes);

	for each(auto mesh in meshList)
	{
		std::set<std::string>::iterator it = importedMeshes.find(mesh->GetExportedFile());
		if (it == importedMeshes.end())
		{
			return true;
		}
	}
	return false;
}

bool ResourceModel::CheckImportedAnimations()
{
	std::set<std::string> importedAnimations;
	App->fsystem->ListFileNames(IMPORTED_ANIMATIONS, importedAnimations);

	for each(auto anim in animationList)
	{
		std::set<std::string>::iterator it = importedAnimations.find(std::to_string(anim->GetUID()));
		if (it == importedAnimations.end())
		{
			return true;
		}
	}
	return false;
}


void ResourceModel::AddMesh(ResourceMesh* mesh)
{
	bool replace = false;
	for (std::vector<ResourceMesh*>::iterator itMesh = meshList.begin(); itMesh != meshList.end();)
	{
		if (mesh->GetUID() == (*itMesh)->GetUID())
		{
			--numMeshes;
			App->resManager->DeleteResourceFromList((*itMesh)->GetUID());
			RELEASE(*itMesh);
			meshList.erase(itMesh++);
			replace = true;
			break;
		}
		itMesh++;
	}

	++numMeshes;
	meshList.push_back(mesh);
}

void ResourceModel::AddAnimation(ResourceAnimation* anim)
{
	bool replace = false;
	for (std::vector<ResourceAnimation*>::iterator itAnim = animationList.begin(); itAnim != animationList.end();)
	{
		if (anim->GetUID() == (*itAnim)->GetUID())
		{
			--numAnimations;
			App->resManager->DeleteResourceFromList((*itAnim)->GetUID());
			RELEASE(*itAnim);
			animationList.erase(itAnim++);
			replace = true;
			break;
		}
		itAnim++;
	}

	++numAnimations;
	animationList.push_back(anim);
}

void ResourceModel::Rename(const char* newName)
{
	// Change name of the scene saved for the model
	std::string sceneFile(App->fsystem->GetFilename(file.c_str()));
	ResourceScene* scene = (ResourceScene*)App->resManager->GetByName(sceneFile.c_str(), TYPE::SCENE);
	if(scene != nullptr)
		scene->Rename(newName);

	Resource::Rename(newName);

	// Update exported file
	exportedFile = App->fsystem->GetFile(file.c_str());

	// Rename meshes
	for (int i = 0; i < meshList.size(); ++i)
	{
		std::string name(newName);
		meshList[i]->Rename((name + "_" + std::to_string(i)).c_str());
		meshList[i]->SetFile(file.c_str());		// Update file variable
	}

	// Rename animations
	for (int i = 0; i < animationList.size(); ++i)
	{
		std::string name(newName);
		animationList[i]->Rename((name + "_" + std::to_string(i)).c_str());
		animationList[i]->SetFile(file.c_str());		// Update file variable
	}
}

void ResourceModel::Delete()
{
	// Delete scene saved for the model
	std::string sceneFile(App->fsystem->GetFilename(file.c_str()));
	ResourceScene* scene = (ResourceScene*)App->resManager->GetByName(sceneFile.c_str(), TYPE::SCENE);
	if (scene != nullptr)
		scene->Delete();

	Resource::Delete();

	// Delete all mesh files
	for each(auto& mesh in meshList)
	{
		mesh->Delete();
	}
	for each(auto& anim in animationList)
	{
		anim->Delete();
	}
}

void ResourceModel::DrawImportConfiguration()
{
	// TODO: [Resource manager] Add import config
}