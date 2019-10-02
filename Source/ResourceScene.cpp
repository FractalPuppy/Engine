#include "ResourceScene.h"

#include "Globals.h"
#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleScene.h"
#include "ModuleSpacePartitioning.h"
#include "ModuleNavigation.h"
#include "ModuleResourceManager.h"
#include "ModuleRender.h"

#include "GameObject.h"
#include "ComponentRenderer.h"
#include "ResourcePrefab.h"

#include "JSON.h"

ResourceScene::ResourceScene(unsigned uid) : Resource(uid, TYPE::SCENE)
{
}

ResourceScene::ResourceScene(const ResourceScene& resource) : Resource(resource)
{
}

ResourceScene::~ResourceScene()
{
}

void ResourceScene::SaveMetafile(const char* file) const
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

	json->AddValue("Scene", *meta);
	filepath += METAEXT;

	// Save meta in Assets
	App->fsystem->Save(filepath.c_str(), json->ToString().c_str(), json->Size());

	// Save meta in Library
	std::string libraryPath(exportedFile + METAEXT);
	App->fsystem->Save(libraryPath.c_str(), json->ToString().c_str(), json->Size());
	RELEASE(json);
}

void ResourceScene::LoadConfigFromMeta()
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
	JSON_value* value = json->GetValue("Scene");

	// Make sure the UID from meta is the same
	unsigned checkUID = value->GetUint("GUID");
	if (oldUID != checkUID)
	{
		UID = checkUID;
		// Update resource UID on resource list
		App->resManager->ReplaceResource(oldUID, this);
		exportedFile = IMPORTED_SCENES + std::to_string(UID) + SCENEEXTENSION;
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

void ResourceScene::LoadConfigFromLibraryMeta()
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
	JSON_value* value = json->GetValue("Scene");

	// Get resource variables
	name = value->GetString("Name");
	file = value->GetString("File");

	RELEASE_ARRAY(data);
	RELEASE(json);
}

void ResourceScene::Save(const GameObject& rootGO, bool selected)
{
	JSON *json = new JSON();
	JSON_value *array = json->CreateValue(rapidjson::kArrayType);
	App->navigation->sceneSaved(json);
	rootGO.Save(array, selected);
	App->scene->selected = nullptr;
	App->scene->selection.clear();
	json->AddValue("GameObjects", *array);

	App->fsystem->Save(file.c_str(), json->ToString().c_str(), json->Size());
	RELEASE(json);
}

bool ResourceScene::Load()
{
	char* data = nullptr;

	if (App->fsystem->Load(exportedFile.c_str(), &data) == 0)
	{
		RELEASE_ARRAY(data);
		return false;
	}

	JSON *json = new JSON(data);
	JSON_value* gameobjectsJSON = json->GetValue("GameObjects");
	std::map<unsigned, GameObject*> gameobjectsMap; //Necessary to assign parent-child efficiently
	GameObject* sceneRoot = new GameObject("World", 0);
	GameObject* sceneCanvas = new GameObject("Canvas", App->scene->canvas->UUID);
	sceneCanvas->parentUUID = -1;
	gameobjectsMap.insert(std::pair<unsigned, GameObject*>(App->scene->canvas->UUID, sceneCanvas));

	std::list<ComponentRenderer*> renderers;
	std::vector<GameObject*> prefabs;

	for (unsigned i = 0; i < gameobjectsJSON->Size(); i++)
	{
		JSON_value* gameobjectJSON = gameobjectsJSON->GetValue(i);
		GameObject *gameobject = new GameObject();
		gameobject->Load(gameobjectJSON);
		if (gameobject->isPrefab && gameobject->isPrefabSync)
		{
			prefabs.emplace_back(gameobject);
		}

		if (gameobject->UUID != 1)
		{
			gameobjectsMap.insert(std::pair<unsigned, GameObject*>(gameobject->UUID, gameobject));

			std::map<unsigned, GameObject*>::iterator it = gameobjectsMap.find(gameobject->parentUUID);
			if (it != gameobjectsMap.end())
			{
				gameobject->parent = it->second;
				gameobject->parent->children.push_back(gameobject);
			}
			else if (gameobject->parentUUID == 0)
			{
				gameobject->parent = sceneRoot;
				sceneRoot->children.push_back(gameobject);
			}

			ComponentRenderer* renderer = nullptr;
			renderer = (ComponentRenderer*)gameobject->GetComponentOld(ComponentType::Renderer);
			if (renderer != nullptr)
			{
				renderers.push_back(renderer);
			}
		}
	}

	if (!App->scene->isCleared()) //Already a scene is loaded
	{
		//Recursive UID reassign
		AssignNewUUID(sceneRoot, 0u);
		for (const auto& child : sceneRoot->children)
		{
			App->scene->root->children.emplace_back(child);
			child->parent = App->scene->root;
		}

		AssignNewUUID(sceneCanvas, 1u);
		for (const auto& child : sceneCanvas->children)
		{
			App->scene->canvas->children.emplace_back(child);
			child->parent = App->scene->canvas;
		}

		sceneRoot->children.clear();
		sceneCanvas->children.clear();
		sceneRoot->CleanUp();
		sceneCanvas->CleanUp();
		RELEASE(sceneRoot);
		RELEASE(sceneCanvas);
	}
	else
	{
		App->scene->root->CleanUp();
		RELEASE(App->scene->root);
		sceneCanvas->parent = sceneRoot;
		sceneRoot->children.emplace_back(sceneCanvas);
		App->scene->root = sceneRoot;
		App->scene->canvas = sceneCanvas;
	}

	//Link Bones after all the hierarchy is imported
	for (ComponentRenderer* cr : renderers)
	{
		if (cr->mesh != nullptr)
		{
			cr->LinkBones();
		}
	}

	for (GameObject* goPrefab : prefabs)
	{
		if (PrefabWasUpdated(goPrefab) || App->scene->SceneIsOlderThanPrefab(goPrefab->prefabUID))
		{
			goPrefab->UpdateToPrefab(goPrefab->prefab->RetrievePrefab());
		}
	}

	App->navigation->sceneLoaded(json);

	RELEASE_ARRAY(data);
	RELEASE(json);

	App->scene->root->OnPlay();
	//set all the game objects
	App->scene->root->UpdateTransforms(math::float4x4::identity);
	App->scene->root->SetAllMoveFlags();
	App->renderer->OnResize();
	return true;
}

void ResourceScene::AssignNewUUID(GameObject* go, unsigned parentUID)
{
	if (go->UUID != 0 && go->UUID != 1)
	{
		go->parentUUID = parentUID;
		go->UUID = App->scene->GetNewUID();
	}

	for (auto& child : go->children)
	{
		AssignNewUUID(child, go->UUID);
	}
}

bool ResourceScene::PrefabWasUpdated(GameObject* prefabGo) const
{
	Resource* prefab = App->resManager->GetWithoutLoad(prefabGo->prefabUID);
	if (prefab == nullptr || prefabGo->prefabTimeStamp == 0u) return false;
	return App->fsystem->GetModTime(prefab->GetFile()) > prefabGo->prefabTimeStamp;
}