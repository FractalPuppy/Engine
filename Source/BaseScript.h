#ifndef  __BaseScript_h__
#define __BaseScript_h__

#include "Component.h"
#include <string>

class Application;
class GameObject;
class JSON_value;
struct ImGuiContext;

class ENGINE_API Script : public Component
{
public:
	Script(const Script& script);
	Script();
	virtual ~Script();

	virtual Script* Clone() const;

	virtual bool CleanUp() override;
	virtual void Enable(bool enable) override;

	void SetApp(Application* app);
	void SetGameObject(GameObject* go);
	void SetGameStandarCursor(std::string gameStandarCursor);
	
	void DrawProperties();

	void Load(JSON_value* json);
	void Save(JSON_value* json) const;
	
	virtual void Expose(ImGuiContext* context);
	virtual void Serialize(JSON_value* json) const {}
	virtual void DeSerialize(JSON_value* json) {}
	virtual void Awake() {}
	virtual void Start() {}
	virtual void Update() {}

	virtual void OnAnimationEvent(std::string name) {}
	//virtual void OnCollisionEnter(GameObject* go) {} //TODO: collision module
	//virtual void OnCollisionExit(GameObject* go) {} //TODO: collision module

	virtual void OnTriggerEnter(GameObject* go) {}
	virtual void OnTrigger(GameObject* go) {}
	virtual void OnTriggerExit(GameObject* go) {}
	virtual void OnButtonDown() {}
	virtual void OnButtonUp() {}

public:
	std::string name = "Script";
	bool hasBeenAwoken = false;
	bool hasBeenStarted = false;
	Application* App = nullptr;
	std::string gameStandarCursor = "Glow.cur";
};

#endif __BaseScript_h__