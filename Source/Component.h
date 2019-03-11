#ifndef __Component_h__
#define __Component_h__

class GameObject;
class JSON_value;

enum class ComponentType 
{
	Transform = 0,
	Renderer,
	Camera,
	Light,
	Script
};

class Component
{
public:
	Component(GameObject* gameobject, ComponentType type);
	Component(const Component& component);

	virtual ~Component();
	virtual Component* Clone() const  = 0;
	
	bool DrawComponentState();

	virtual void DrawProperties() = 0;
	virtual void Enable()
	{
		enabled = true;
	}

	virtual void Update() {}
	virtual void Disable()
	{
		enabled = false;
	}
	
	virtual bool CleanUp() 
	{
		return true; 
	}

	void Remove();

	virtual void Save(JSON_value* value) const;
	virtual void Load(JSON_value* value);

public:
	GameObject* gameobject = nullptr;
	ComponentType type;
	bool enabled = true;
};

#endif __Component_h__