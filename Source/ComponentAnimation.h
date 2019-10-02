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

#define MAX_STATEMACHINE_NAME 30

class StateMachine;

class ComponentAnimation : public Component
{
public:
	typedef ax::NodeEditor::EditorContext EditorContext;

	void DrawProperties();
	Component* Clone() const;

	void SetStateMachine(const char* stateMachineFile);
	ENGINE_API void SendTriggerToStateMachine(const char* trigger);
	ResourceAnimation* GetAnimFromStateMachine();
	bool GetLoopFromStateMachine();
	ENGINE_API float GetDurationFromClip();
	float GetSpeedFromStateMachine();
	bool GetMustFinishFromStateMachine();
	void PlayNextNode(float blend);

	void SetIndexChannels(GameObject* GO, ResourceAnimation* Ranim);
	void UpdateGO(GameObject* gameobject);
	void EditorUpdateGO(GameObject* gameobject);
	void ResetResource();
	void CreateNewStateMachine();

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

	unsigned currentNode = 0u;

public:
	AnimationController* editorController = nullptr;
	AnimationController* controller = nullptr;
	ResourceStateMachine* stateMachine = nullptr;

	EditorContext* context = nullptr;
	char* animName = new char[64];
	bool isPlaying = false;

	std::vector<std::string> guiAnimations;
	std::vector<std::string> guiStateMachines;

	char newStMachineName[MAX_STATEMACHINE_NAME] = "";	// Aux variable for state machine rename
};

#endif //  __COMPONENTANIMATION_H_
