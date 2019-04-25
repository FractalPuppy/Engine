#ifndef __COMPONENTANIMATION_H_
#define __COMPONENTANIMATION_H_

#include "Component.h"
#include "ResourceAnimation.h"
#include "Resource.h"
#include "AnimationController.h"
#include "ResourceStateMachine.h"
#include "NodeEditor.h"

#include "HashString.h"
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
	void SetStateMachine(const char* stateMachineFile);
	void SendTriggerToStateMachine(HashString trigger);
	ResourceAnimation* GetAnimFromStateMachine();
	bool GetLoopFromStateMachine();
	void PlayNextNode(unsigned blend);

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

	EditorContext* GetEditorContext();
	void Update() override;
	void OnPlay() override;
	bool CleanUp();
public:

	unsigned clipIndexToRemove = 0u;
	bool clipRemove = false;
	bool stateMachineDelete = false;
	bool channelsSetted = false;
	bool deletePopup = false;


public:
	AnimationController* controller = nullptr;
	ResourceAnimation* anim = nullptr;
	ResourceStateMachine* stateMachine = nullptr;

	EditorContext* context = nullptr;
	char* animName = new char[64];
	bool isPlaying = false;

	std::vector<std::string> guiAnimations;
	std::vector<std::string> guiStateMachines;
};

#endif //  __COMPONENTANIMATION_H_
