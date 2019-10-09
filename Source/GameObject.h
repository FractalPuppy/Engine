#ifndef __GameObject_h__
#define __GameObject_h__

#include <vector>
#include <list>
#include "Geometry/AABB.h"
#include "Math/float4x4.h"
#include "Globals.h"

class Component;
class ComponentTransform;
class ComponentLight;
class ComponentAnimation;
class ComponentText;
class ComponentRenderer;
class AABBTreeNode;
class ResourcePrefab;
enum class ComponentType;
struct Frame;
struct Texture;
class JSON_value;
class Animation;

class GameObject
{
public:
	GameObject() {};
	GameObject(const char* name, unsigned uuid);
	GameObject(const float4x4& transform, const char* name, unsigned uuid);
	ENGINE_API  GameObject(const GameObject& gameobject);
	~GameObject();

	void DrawProperties();

	ENGINE_API void Update();
	void PreUpdate();
	ENGINE_API inline bool isActive() const
	{
		if (!activeInHierarchy) return false;
		return activeSelf;
	}

	ENGINE_API void SetActive(bool active);

	ENGINE_API Component* CreateComponent(ComponentType type, JSON_value* value = nullptr, bool prefabTemplate = false);

	template <class T>
	T* GetComponent() const;
	ENGINE_API Component* GetComponentOld(ComponentType type) const;
	ENGINE_API Component* GetComponentInChildren(ComponentType type) const;

	ENGINE_API std::vector<Component *> GetComponents(ComponentType type) const;
	ENGINE_API std::vector<Component *> GetComponentsInChildren(ComponentType type) const;
	void RemoveComponent(const Component & component);
	ENGINE_API void RemoveChild(GameObject* child);
	ENGINE_API void InsertChild(GameObject* child);
	ENGINE_API void ChangeParent(GameObject * oldParent, GameObject * newParent);
	bool IsParented(const GameObject & gameobject) const;

	void DrawHierarchy();

	ENGINE_API void UpdateGlobalTransform();
	ENGINE_API void SetGlobalTransform(const float4x4 &global);
	ENGINE_API float4x4 GetGlobalTransform() const;
	ENGINE_API float4x4 GetLocalTransform() const;

	ENGINE_API void OnPlay();
	void SetAllMoveFlags();
	ENGINE_API void UpdateTransforms(math::float4x4 parentGlobal);
	bool CheckDelete();

	void UpdateBBox();
	void DrawBBox() const;
	ENGINE_API math::AABB GetBoundingBox() const;
	bool Intersects(const LineSegment & line, float & distance, math::float3* intersectionPoint = nullptr) const;
	ENGINE_API bool BboxIntersects(const GameObject* target) const;
	void UpdateModel(unsigned int shader) const;
	void SetLightUniforms(unsigned shader) const;
	void LinkRendererToBones(std::vector<ComponentRenderer*>& renderers);

	ENGINE_API void LinkBones() const;

	void UpdateToPrefab(GameObject* prefab);
	bool ChildPrefab() const;
	bool ParentPrefab() const;

	bool CleanUp();
	void Save(JSON_value *gameobjects, bool selected = false) const;
	void Load(JSON_value * gameobject, bool prefabObject= false);

private:
	void SetStaticAncestors();
	void MarkAsPrefab();
	void SetActiveInHierarchy(bool active);
	void OnChangeActiveState(bool wasActive);
	void MakeObjectWithMeshStatic();

	void SetStaticAllChildsWithMesh();
	void SetNavigableAllChildsWithMesh();
	void SetObstacleAllChildsWithMesh();
	void AddAllNavigableChildsToNavMesh();

	bool activeInHierarchy = true;
	bool openInHierarchy = true;
	bool showNavOptions = false;
public:
	unsigned UUID = 0;
	unsigned parentUUID = 0; //only set in Save/Load scene TODO:update on parent change
	unsigned animationIndexChannel = 999u;
	bool isStatic = false;
	bool isBoneRoot = false;
	bool activeSelf = true;

	bool isPrefabSync = false;
	ResourcePrefab* prefab = nullptr;
	bool isPrefab = false;
	unsigned prefabUID = 0;
	bool isDropablePlaceHolder = false;
	unsigned prefabTimeStamp = 0u;

	bool navigable = false;
	bool noWalkable = false;
	bool isSelected = false;
	bool movedFlag = false;
	bool copyFlag = false;
	bool deleteFlag = false;
	bool drawBBox = false;
	bool particlesDirty = false;

	math::AABB bbox;

	math::float4x4 baseState = math::float4x4::identity;

	ComponentTransform * transform = nullptr;
	GameObject *parent = nullptr;
	
	//ABBTree Stuff
	AABBTreeNode *treeNode = nullptr; //Direct reference to the aabtree node holding the gameobject
	bool isVolumetric = false; //Indicates if the gameobject has a mesh
	//
	bool hasLight = false;
	ComponentLight* light = nullptr;

	std::vector<Component*> components;
	std::list<GameObject*> children;	

	std::string name = "GameObject";
	std::string tag = "Default";

};

template<class T>
T* GameObject::GetComponent() const
{
	for (auto &component : components)
	{
		if (T* c = dynamic_cast<T*>(component))
			return c;
	}
	return nullptr;
}
#endif __GameObject_h__