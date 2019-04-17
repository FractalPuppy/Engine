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
class AABBTreeNode;
enum class ComponentType;
struct Frame;
struct Texture;
class JSON_value;
class Animation;
class Script;

class GameObject
{
public:
	GameObject() {};
	GameObject(const char* name, unsigned uuid);
	GameObject(const float4x4& transform, const char* name, unsigned uuid);
	GameObject(const GameObject& gameobject);
	~GameObject();

	void DrawProperties();

	void Update();
	inline bool isActive() const
	{
		if (!activeInHierarchy) return false;
		return activeSelf;
	}

	ENGINE_API void SetActive(bool active);

	Component* CreateComponent(ComponentType type);
	ENGINE_API Component* GetComponent(ComponentType type) const;
	ENGINE_API Component* GetComponentInChildren(ComponentType type) const;

	ENGINE_API Script* GetScript() const; //Returns first script found in GameObject
	ENGINE_API Script* FindScriptByName(const char* name) const;

	ENGINE_API std::vector<Component *> GetComponents(ComponentType type) const;
	ENGINE_API std::vector<Component *> GetComponentsInChildren(ComponentType type) const;
	void RemoveComponent(const Component & component);
	void RemoveChild(GameObject* child);
	void InsertChild(GameObject* child);
	bool IsParented(const GameObject & gameobject) const;

	void DrawHierarchy();

	void UpdateGlobalTransform();
	void SetGlobalTransform(const float4x4 &global);
	float4x4 GetGlobalTransform() const;
	float4x4 GetLocalTransform() const;

	void UpdateTransforms(math::float4x4 parentGlobal);
	bool CheckDelete();

	void UpdateBBox();
	void DrawBBox() const;
	math::AABB GetBoundingBox() const;
	bool Intersects(const LineSegment & line, float & distance) const;
	ENGINE_API bool BboxIntersects(const GameObject* target) const;
	void UpdateModel(unsigned int shader) const;
	void SetLightUniforms(unsigned shader) const;

	bool CleanUp();
	void Save(JSON_value *gameobjects) const;
	void Load(JSON_value * gameobject);

private:
	void SetStaticAncestors();

public:
	unsigned UUID = 0;
	unsigned parentUUID = 0; //only set in Save/Load scene TODO:update on parent change
	unsigned animationIndexChannel = 999u;
	bool isStatic = false;
	bool isBoneRoot = false;
	bool activeInHierarchy = true;
	bool activeSelf = true;

	bool isSelected = false;
	bool movedFlag = false;
	bool copyFlag = false;
	bool deleteFlag = false;
	bool drawBBox = false;

	math::AABB bbox;

	math::float4x4 baseState = math::float4x4::identity;

	ComponentTransform * transform = nullptr;
	GameObject *parent = nullptr;
	
	//ABBTree Stuff
	AABBTreeNode *treeNode = nullptr; //Direct reference to the aabtree node holding the gameobject
	bool isVolumetric = false; //Indicates if the gameObject has a mesh
	//
	bool hasLight = false;
	ComponentLight* light = nullptr;

	std::vector<Component*> components;
	std::list<GameObject*> children;	

	std::string name = "GameObject";

};

#endif __GameObject_h__

