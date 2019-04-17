#ifndef __COMPONENTANIMATION_H_
#define __COMPONENTANIMATION_H_

#include "Component.h"
#include "ResourceAnimation.h"
#include "Resource.h"
#include "AnimationController.h"
#include "StateMachine.h"
#include "NodeEditor.h"

#include "Math/float4x4.h"
#include "Math/float3.h"
#include "Math/Quat.h"

class StateMachine;

class ComponentAnimation : public Component
{
public:
	typedef ax::NodeEditor::EditorContext EditorContext;


	void DrawProperties();
	Component* Clone() const;

	void SetAnimation(const char* animationFile);

	void SetIndexChannels(GameObject* GO);
	void PlayAnimation(unsigned blend);
	void UpdateGO(GameObject* gameobject);
	void ResetResource();

	ComponentAnimation();
	ComponentAnimation(GameObject* gameobject);
	ComponentAnimation(const ComponentAnimation &copy);
	~ComponentAnimation();

	void Save(JSON_value* value) const override;
	void Load(JSON_value* value) override;

	void Update() override;
	bool CleanUp();
public:

	bool channelsSetted = false;


public:
	AnimationController* controller = nullptr;
	ResourceAnimation* anim = nullptr;

	char* animName = new char[64];
	bool isPlaying = false;

	std::vector<std::string> guiAnimations;
};

#endif //  __COMPONENTANIMATION_H_
