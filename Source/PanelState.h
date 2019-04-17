#ifndef __PANELSTATE_H_
#define __PANELSTATE_H_

#include "Panel.h"

#include "NodeEditor.h"

namespace ed = ax::NodeEditor;

class StateMachine;

enum class PinKind
{
	Output,
	Input
};

class PanelState : public Panel
{
public:
	PanelState();
	~PanelState();

	ax::NodeEditor::EditorContext* context = nullptr;

	void Draw() override;

};

#endif // __PANELSTATE_H_