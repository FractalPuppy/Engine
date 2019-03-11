#ifndef  __BaseScript_h__
#define __BaseScript_h__

class Application;
class GameObject;
class JSON_value;
struct ImGuiContext;

#ifdef BASESCRIPT_EXPORTS
#define  BASESCRIPT_API __declspec(dllexport)
#else
#define BASESCRIPT_API __declspec(dllimport)
#endif

class BASESCRIPT_API Script
{
public:
	void SetApp(Application* app);
	void SetGameObject(GameObject* go);
	virtual void Expose(ImGuiContext* context);
	virtual void Serialize(JSON_value* json) const {}
	virtual void DeSerialize(JSON_value* json) {}
	virtual void Start() {}
	virtual void Update() {}

protected:
	GameObject* gameObject = nullptr;
	Application* App = nullptr;

};

#endif __BaseScript_h__