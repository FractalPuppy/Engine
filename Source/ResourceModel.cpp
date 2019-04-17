#include "ResourceModel.h"

#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h"

#include "ResourceMesh.h"
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
	DeleteFromMemory();
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
}

void ResourceModel::LoadConfigFromMeta()
{
	Resource::LoadConfigFromMeta();

	char* data = nullptr;
	std::string metaFile(file);
	metaFile += ".meta";

	if (App->fsystem->Load(metaFile.c_str(), &data) == 0)
	{
		LOG("Warning: %s couldn't be loaded", metaFile.c_str());
		RELEASE_ARRAY(data);
		return;
	}
	JSON* json = new JSON(data);
	JSON_value* value = json->GetValue("Mesh");
	UID = value->GetUint("GUID");
	numMeshes = value->GetUint("NumMeshes");
	std::string name = App->fsystem->GetFilename(file);

	for (int i = 0; i < numMeshes; ++i)
	{
		unsigned meshUID = value->GetUint(("Mesh" + std::to_string(i)).c_str());
		ResourceMesh* mesh = (ResourceMesh*)App->resManager->CreateNewResource(TYPE::MESH, meshUID);
		mesh->SetFile(file.c_str());
		mesh->SetExportedFile((std::to_string(mesh->GetUID())).c_str());
		mesh->name = name + "_" + std::to_string(i);

		meshList.push_back(mesh);
	}

	value = json->GetValue("Animation");
	numAnimations = value->GetUint("NumAnimations");

	for (int i = 0; i < numAnimations; ++i)
	{
		unsigned animUID = value->GetUint(("Animation" + std::to_string(i)).c_str());
		ResourceAnimation* anim = (ResourceAnimation*)App->resManager->CreateNewResource(TYPE::ANIMATION, animUID);
		anim->SetFile(file.c_str());
		anim->SetExportedFile((std::to_string(anim->GetUID())).c_str());
		anim->name = name + "_" + std::to_string(i);

		animationList.push_back(anim);
	}

}

bool ResourceModel::CheckImportedMeshes()
{
	std::set<std::string> importedMeshes;
	App->fsystem->ListFiles(MESHES, importedMeshes);

	for each(auto mesh in meshList)
	{
		std::set<std::string>::iterator it = importedMeshes.find(std::to_string(mesh->GetUID()));
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
	App->fsystem->ListFiles(MESHES, importedAnimations);

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
	Resource::Rename(newName);

	exportedFileName = newName;

	for (int i = 0; i < meshList.size(); ++i)
	{
		std::string name(newName);
		meshList[i]->Rename((name + "_" + std::to_string(i)).c_str());
	}
}

void ResourceModel::Delete()
{
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