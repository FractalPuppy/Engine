#ifndef __ModuleScene_h__
#define __ModuleScene_h__

#include "Module.h"
#include "ComponentLight.h"

#include "Geometry/Frustum.h"
#include "pcg-cpp-0.98/include/pcg_random.hpp"
#include "Math/Quat.h"
#include "Math/float4.h"
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
class myQuadTree;
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

	GameObject * CreateGameObject(const char * name, GameObject* parent);

	void AddToSpacePartition(GameObject * gameobject);
	void DeleteFromSpacePartition(GameObject* gameobject);
	void ResetQuadTree(); //deprecated

	void FrustumCulling(const Frustum &frustum);
	void Draw(const Frustum &frustum, bool isEditor = false);
	void DrawGOGame(const GameObject& go);
	void DrawGO(const GameObject& go, const Frustum & frustum, bool isEditor = false);
	void DrawHierarchy();
	void DragNDropMove(GameObject* target) ;
	void DragNDrop(GameObject * go);
	void DrawGUI() override;

	void CreateCube(const char * name, GameObject* parent);
	void CreateSphere(const char * name, GameObject* parent);
	void CreatePrimitive(const char * name, GameObject* parent, PRIMITIVES type);
	void SetPrimitiveMesh(par_shapes_mesh_s * mesh, PRIMITIVES type);
	unsigned SaveParShapesMesh(const par_shapes_mesh_s & mesh, char** data) const;

	void SaveScene(const GameObject &rootGO, const char* scene, const char* scenePath);
	void TakePhoto();
	void TakePhoto(std::list<GameObject*>& target);
	void RestorePhoto(GameObject* photo);
	void RestoreLastPhoto();
	void Redo();
	ENGINE_API void LoadScene(const char* scene, const char* path);
	bool AddScene(const char* scene, const char* scenePath);								// Adds a scene to current opened scene from a scene file (returns true if it was loaded correctly)

	void ClearScene();

	void Select(GameObject* gameobject);
	void UnSelect();
	void Pick(float normalized_x, float normalized_y);

	ENGINE_API GameObject * FindGameObjectByName(const char* name) const;
	ENGINE_API GameObject * FindGameObjectByName(GameObject* parent, const char* name) const;

	void GetStaticGlobalAABB(math::AABB &aabb, std::vector<GameObject*> &bucket, unsigned int &bucketOccupation);

	unsigned GetNewUID();
	std::list<ComponentLight*> GetClosestLights(LightType type, math::float3 position = math::float3::zero) const;

	ComponentLight * GetDirectionalLight() const;

private:
	std::list<std::pair<float, GameObject*>>GetDynamicIntersections(const LineSegment& line);
	std::list<std::pair<float, GameObject*>>GetStaticIntersections(const LineSegment& line);
	unsigned primitivesUID[NBPRIMITIVES] = {0};
	std::unordered_set<GameObject*> dynamicFilteredGOs;
	std::unordered_set<GameObject*> staticFilteredGOs;

	std::list<GameObject*> scenePhotos;
	std::list<GameObject*> scenePhotosUndoed;

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
	pcg32 uuid_rng;
	std::string name;
	std::string path;
	std::string defaultScene;
	bool photoEnabled = false;
	float photoTimer = 0.f;
	float3 ambientColor = float3::one;

	int SceneSize = 10000;

	GameObject* canvas = nullptr;
};



#endif __ModuleScene_h__