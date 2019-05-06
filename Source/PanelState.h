#ifndef __PANELSTATE_H_
#define __PANELSTATE_H_

#include "Panel.h"

#include "imgui.h"

#include "NodeEditor.h"
#include "HashString.h"

namespace ed = ax::NodeEditor;

class ResourceStateMachine;
class ComponentAnimation;

class PanelState : public Panel
{
public:
	PanelState();
	~PanelState();

	void DrawSM(ResourceStateMachine* stateMachine, ax::NodeEditor::EditorContext* context);
	void DrawNodes(ResourceStateMachine* stateMachine);
	void DrawTransitions(ResourceStateMachine* stateMachine);
	void ShowContextMenus(ResourceStateMachine* stateMachine);
	void ShowNodeMenu(ResourceStateMachine* stateMachine);
	void ShowTransitionMenu(ResourceStateMachine* stateMachine);
	void ShowCreateNewNodeMenu(ResourceStateMachine* stateMachine);
	void ManageCreate(ResourceStateMachine* stateMachine);
	void AddNode(ResourceStateMachine* stateMachine, HashString nodeName, HashString clipName);
	void Draw() override;

	bool focus = false;
private:
	unsigned contextNode = 0u;
	unsigned contextLink = 0u;

	ImVec2 newNodePosition;
	ed::PinId newNodePin = 0;
};

#endif // __PANELSTATE_H_