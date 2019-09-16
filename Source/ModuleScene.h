#ifndef __ModuleScene_h__
#define __ModuleScene_h__

#include "Module.h"
#include "ComponentLight.h"

#include "Geometry/Frustum.h"
#include "pcg-cpp-0.98/include/pcg_random.hpp"
#include "Math/Quat.h"
#include "Math/float4.h"
#include "Math/float2.h"
#include "Geometry/LineSegment.h"
#include "SDL/include/SDL_timer.h"
#include <set>
#include <unordered_set>
#include <string>
#include <thread>
#include <mutex>

#define NBPRIMITIVES 2
#define TIME_BETWEEN_PHOTOS 1000.f
#define MAX_PHOTOS 10

class GameObject;
class ComponentCamera;
class ComponentLight;
class ResourceTexture;
class ResourceScene;
class myQuadTree;
class ResourcePrefab;
class ComponentRenderer;

struct par_shapes_mesh_s;

enum class PRIMITIVES
{
	SPHERE,
	CUBE,
};

class ModuleScene :
	public Module
{
public:

	ModuleScene();
	~ModuleScene();

	bool Init(JSON * config) override;
	bool Start() override;
	update_status PreUpdate() override;
	update_status Update(float dt) override;
	bool CleanUp() override;
	void SaveConfig(JSON* config) override;

	GameObject* CreateGameObject(const char * name, GameObject* parent);

	void AddToSpacePartition(GameObject * gameobject);
	void DeleteFromSpacePartition(GameObject* gameobject);
	void ResetQuadTree(); //deprecated


	void FrustumCulling(const Frustum &frustum);
	void Draw(const Frustum &frustum, bool isEditor = false);
	void DrawGO(const GameObject& go, const Frustum & frustum, bool isEditor = false);
	void DrawHierarchy();
	void DragNDropMove(GameObject* target) ;
	void DragNDrop(GameObject * go);
	void DrawGUI() override;

	bool SceneIsOlderThanPrefab(unsigned UID) const;
	unsigned CreatePrefab(GameObject* go);
	void CreateCube(const char * name, GameObject* parent);
	void CreateSphere(const char * name, GameObject* parent);
	void CreatePrimitive(const char * name, GameObject* parent, PRIMITIVES type);
	void SetPrimitiveMesh(par_shapes_mesh_s * mesh, PRIMITIVES type);
	unsigned SaveParShapesMesh(const par_shapes_mesh_s & mesh, char** data) const;

	void SaveScene(const GameObject& rootGO, const char* sceneName, const char* folder, bool selected = false);
	void SaveTemporaryScene();
	bool isCleared();
	ENGINE_API void LoadScene(const char* sceneName, const char* folder);
	void LoadTemporaryScene();
	bool AddScene(const char* sceneName, const char* folder);								// Adds a scene to current opened scene from a scene file (returns true if it was loaded correctly)

	void AssignNewUUID(GameObject* go, unsigned UID);
	void TakePhoto();
	void TakePhoto(std::list<GameObject*>& target);
	void RestorePhoto(GameObject* photo);
	void RestoreLastPhoto();
	void Redo();

	void ClearScene();
	void UpdateScenesList();

	void Select(GameObject* gameobject);
	void UnSelect();
	void Pick(float normalized_x, float normalized_y);
	ENGINE_API bool Intersects(math::float3& closestPoint, const char* name, bool editor = false);
	ENGINE_API bool Intersects(const char* tag, bool sorted, math::float3& intersection, GameObject** out = nullptr) const;

	GameObject* FindClosestParent(GameObject* go);

	ENGINE_API GameObject* FindGameObjectByTag(const char* tag, GameObject* parent = nullptr) const;
	ENGINE_API std::vector<GameObject*> FindGameObjectsByTag(const char * tag, GameObject* parent = nullptr) const;
	ENGINE_API GameObject* FindGameObjectByUID(unsigned UID, GameObject* parent = nullptr) const;
	ENGINE_API GameObject* FindGameObjectByName(const char* name, GameObject* parent = nullptr) const;

	ENGINE_API GameObject* Spawn(const char* name, GameObject* parent= nullptr);
	ENGINE_API GameObject* Spawn(const char* name, math::float3 position,
		math::Quat rotation, GameObject* parent = nullptr);
		
	void GetStaticGlobalAABB(math::AABB &aabb, std::vector<GameObject*> &bucket, unsigned int &bucketOccupation);

	unsigned GetNewUID();
	std::list<ComponentLight*> GetClosestLights(LightType type, math::float3 position = math::float3::zero) const;

	ComponentLight* GetDirectionalLight() const;
	void DeleteDirectionalLight(ComponentLight* light);

	ENGINE_API math::LineSegment SceneRaycast(math::float2 position);
	ENGINE_API std::list<GameObject*> SceneRaycastHit(math::float2 position);

private:
	std::list<std::pair<float, GameObject*>>GetDynamicIntersections(const LineSegment& line, bool sort = true) const;
	std::list<std::pair<float, GameObject*>>GetStaticIntersections(const LineSegment& line) const;
	unsigned primitivesUID[NBPRIMITIVES] = {0};

	std::list<GameObject*> scenePhotos;
	std::list<GameObject*> scenePhotosUndoed;

	std::list<Component*> alphaRenderers;

	unsigned defaultSceneUID = 0u;
	std::vector<std::string> sceneFiles;
	
public:
	GameObject* root = nullptr;
	GameObject* selected = nullptr; //Selected in hierarchy
	Component* copyComp = nullptr; // Copied values in inspector
	ComponentCamera* maincamera = nullptr; //Released by GameObject holding it
	ResourceTexture* camera_notfound_texture = nullptr; //Released in resource manager
	std::list<LineSegment> debuglines;
	std::list<GameObject*> selection;
	std::list<ComponentLight*> lights;
	myQuadTree * quadtree = nullptr;
	std::set<GameObject*> dynamicGOs;
	std::set<GameObject*> staticGOs;
	std::unordered_set<GameObject*> dynamicFilteredGOs;
	std::unordered_set<GameObject*> staticFilteredGOs;
	pcg32 uuid_rng;
	std::string name;
	std::string path;
	ResourceScene* defaultScene = nullptr;
	bool photoEnabled = false;
	float photoTimer = 0.f;
	float3 ambientColor = float3::one;

	int SceneSize = 10000;

	GameObject* canvas = nullptr;

	bool loadScene = false;
	bool actionAfterLoad = false;
	std::string stateAfterLoad = "None";

	//variable that tells us if an enemy is hovered
	struct enemyInfo
	{
		GameObject* object = nullptr;
		int health = 0;
	};
	enemyInfo enemyHovered;
	
};



#endif __ModuleScene_h__