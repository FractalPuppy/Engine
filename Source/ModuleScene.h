#ifndef __ModuleScene_h__
#define __ModuleScene_h__

#include "Module.h"
#include "ComponentLight.h"

#include "Geometry/Frustum.h"
#include "pcg_random.hpp"
#include "Math/Quat.h"
#include "Math/float4.h"
#include <set>
#include <unordered_set>

#define NBPRIMITIVES 2
class GameObject;
class ComponentCamera;
class ComponentLight;
class myQuadTree;
struct Texture;
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
	void SaveConfig(JSON * config) override;

	GameObject * CreateGameObject(const char * name, GameObject* parent);

	void AddToSpacePartition(GameObject * gameobject);
	void DeleteFromSpacePartition(GameObject & gameobject);
	void ResetQuadTree(); //deprecated

	void FrustumCulling(const Frustum &frustum);
	void Draw(const Frustum &frustum, bool isEditor = false);
	void DrawGO(const GameObject& go, const Frustum & frustum, bool isEditor = false);
	void DrawHierarchy();
	void DragNDrop(GameObject * go);
	void DrawGUI() override;

	void CreateCube(const char * name, GameObject* parent);
	void CreateSphere(const char * name, GameObject* parent);
	void CreatePrimitive(const char * name, GameObject* parent, PRIMITIVES type);
	void SetPrimitiveMesh(par_shapes_mesh_s * mesh, PRIMITIVES type);
	unsigned SaveParShapesMesh(const par_shapes_mesh_s & mesh, char** data) const;

	void SaveScene(const GameObject &rootGO, const char& scene, const char& scenePath);
	void LoadScene(const char& scene, const char& path);
	bool AddScene(const char& scene, const char& scenePath);								// Adds a scene to current opened scene from a scene file (returns true if it was loaded correctly)
	void ClearScene();

	void Select(GameObject* gameobject);
	void UnSelect();
	void Pick(float normalized_x, float normalized_y);

	void GetStaticGlobalAABB(AABB &aabb, std::vector<GameObject*> &bucket, unsigned int &bucketOccupation);

	unsigned GetNewUID();
	std::list<ComponentLight*> GetClosestLights(LightType type, float3 position = float3::zero) const;

	ComponentLight * GetDirectionalLight() const;

private:
	std::list<std::pair<float, GameObject*>>GetDynamicIntersections(const LineSegment& line);
	std::list<std::pair<float, GameObject*>>GetStaticIntersections(const LineSegment& line);
	unsigned primitivesUID[NBPRIMITIVES] = {0};
	std::unordered_set<GameObject*> dynamicFilteredGOs;
	std::unordered_set<GameObject*> staticFilteredGOs;

public:
	GameObject* root = nullptr;
	GameObject* selected = nullptr; //Selected in hierarchy
	ComponentCamera* maincamera = nullptr; //Released by GameObject holding it
	Texture* camera_notfound_texture = nullptr; //Released in resource manager
	std::list<LineSegment> debuglines;

	std::list<ComponentLight*> lights;
	myQuadTree * quadtree = nullptr;
	std::set<GameObject*> dynamicGOs;
	std::set<GameObject*> staticGOs;
	pcg32 uuid_rng;
	std::string name;
	std::string path;
	std::string defaultScene;

	float3 ambientColor = float3::one;
};

#endif __ModuleScene_h__