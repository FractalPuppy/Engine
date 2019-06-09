#include "ResourcePrefab.h"
#include "GL/glew.h"

#include "Globals.h"
#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h"
#include "ModuleScene.h"
#include "GameObject.h"
#include "ComponentRenderer.h"
#include "ComponentTransform.h"

#include "JSON.h"

ResourcePrefab::ResourcePrefab(unsigned uid) : Resource(uid, TYPE::PREFAB)
{
}

ResourcePrefab::ResourcePrefab(const ResourcePrefab& resource) : Resource(resource)
{
}

ResourcePrefab::~ResourcePrefab()
{
	DeleteFromMemory();
}

bool ResourcePrefab::LoadInMemory()
{
	char* data = nullptr;

	unsigned ok = App->fsystem->Load((IMPORTED_PREFABS + std::to_string(UID) + PREFABEXTENSION).c_str(), &data);

	// Load prefab
	if (ok != 0)			
	{
		++loaded;
		Load(&data);
	}
	return true;
}

void ResourcePrefab::DeleteFromMemory()
{
	Resource::DeleteFromMemory();
	if (root != nullptr)
	{
		root->treeNode = nullptr;
		root->CleanUp();
		RELEASE(root);
	}
	if (prefabJson != nullptr)
	{
		RELEASE(prefabJson);
		prefabValue = nullptr;
	}
}

void ResourcePrefab::SaveMetafile(const char * file) const
{
	JSON* json = new JSON();
	JSON_value* meta = json->CreateValue();
	struct stat statFile;
	stat(file, &statFile);
	meta->AddUint("GUID", UID);
	meta->AddUint("timeCreated", statFile.st_ctime);
	std::string filepath(file);
	filepath += METAEXT;

	json->AddValue("Prefab", *meta);

	App->fsystem->Save(filepath.c_str(), json->ToString().c_str(), json->Size());
	RELEASE(json);
}

void ResourcePrefab::LoadConfigFromMeta()
{
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
	JSON_value* value = json->GetValue("Prefab");
	UID = value->GetUint("GUID");
	RELEASE(data);
}


void ResourcePrefab::Load(char** data)
{
	prefabJson = new JSON(*data);
	prefabValue = prefabJson->GetValue("GameObjects");
	RELEASE_ARRAY(*data);
}

void ResourcePrefab::Save(GameObject* go) const
{
	JSON *json = new JSON();
	JSON_value *array = json->CreateValue(rapidjson::kArrayType);
	go->Save(array);
	json->AddValue("GameObjects", *array);

	App->fsystem->Save(GetFile(), json->ToString().c_str(), json->Size());
	RELEASE(json);
}

void ResourcePrefab::AddInstance(GameObject* go)
{
	instances.push_back(go);
}

bool ResourcePrefab::RemoveInstance(GameObject* go)
{
	for (std::vector<GameObject*>::iterator itChild = instances.begin(); itChild != instances.end(); ++itChild)
	{
		if (go == *itChild)
		{
			instances.erase(itChild);
			return true;
		}
	}
	return false;
}

unsigned ResourcePrefab::GetNbInstances() const
{
	return instances.size();
}

void ResourcePrefab::Update(GameObject* go)
{
	for (auto& instance : instances)
	{
		if (instance == go || !instance->isPrefabSync) continue;
		instance->UpdateToPrefab(go);
	}
	Save(go);
}

GameObject* ResourcePrefab::RetrievePrefab()
 {
	if (root != nullptr)
	{
		root->CleanUp();
		RELEASE(root);
	}

	std::map<unsigned, GameObject*> gameobjectsMap;

	std::list<ComponentRenderer*> renderers;

	unsigned size = prefabValue->Size();
	for (unsigned i = 0; i < size; i++)
	{
		JSON_value* gameobjectJSON = prefabValue->GetValue(i);
		GameObject *gameobject = new GameObject();
		gameobject->Load(gameobjectJSON, true);
		if (gameobject->UUID != 1 && gameobject->UUID != 0)
		{
			gameobjectsMap.insert(std::pair<unsigned, GameObject*>(gameobject->UUID, gameobject));
			std::map<unsigned, GameObject*>::iterator it = gameobjectsMap.find(gameobject->parentUUID);
			if (it != gameobjectsMap.end())
			{
				gameobject->parent = it->second;
				gameobject->parent->children.push_back(gameobject);
			}
			else //Case root gameobject
			{
				root = gameobject;
			}
		}
		else
		{
			root = gameobject;
		}
		root->isPrefabSync = true;
		root->isPrefab = true;
		root->prefabUID = UID;
		root->transform->Reset();

		ComponentRenderer* renderer = nullptr;
		renderer = (ComponentRenderer*)gameobject->GetComponentOld(ComponentType::Renderer);
		if (renderer != nullptr)
		{
			renderers.push_back(renderer);
		}

		App->scene->DeleteFromSpacePartition(gameobject);
	}

	for (ComponentRenderer* cr : renderers)
	{
		if (cr->mesh != nullptr)
		{
			cr->LinkBones();
		}
	}

	return root;
}
