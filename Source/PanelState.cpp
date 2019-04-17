#include "PanelState.h"

#include "imgui.h"
#include "NodeEditor.h"

//#define IMGUI_DEFINE_MATH_OPERATORS
//#include "imgui_internal.h"

PanelState::PanelState()
{
}

PanelState::~PanelState()
{
	/*if (context)
	{
		ed::DestroyEditor(context);
		context = nullptr;
	}*/
}

void PanelState::Draw()
{
	if (!ImGui::Begin("State Machine", &enabled))
	{
		ImGui::End();
		return;
	}

	//ed::Begin("State Machine Editor", ImVec2(0.0, 0.0f));

	//ed::PushStyleColor(ed::StyleColor_PinRect, ImColor(60, 180, 255, 150));
	//ed::PushStyleColor(ed::StyleColor_PinRectBorder, ImColor(60, 180, 255, 150));
	//ed::BeginNode(1);
	//ImGui::Indent(1.0);
	//ImGui::TextColored(ImVec4(255, 255, 0, 255),"Test");
	//// In Pin
	//ed::PushStyleVar(ed::StyleVar_PinArrowSize, 8.0f);
	//ed::PushStyleVar(ed::StyleVar_PinArrowWidth, 8.0f);
	//ed::PushStyleVar(ed::StyleVar_PinRadius, 10.0f);
	//ed::PushStyleVar(ed::StyleVar_TargetDirection, ImVec2(0.0f, 0.0f));
	//ed::BeginPin(2, ed::PinKind::Input);
	//ImGui::Text("In");
	//ed::EndPin();
	//ed::PopStyleVar(4);
	//// Out Pin
	//ImGui::SameLine(40 - 40);
	//ed::PushStyleVar(ed::StyleVar_PinArrowSize, 0.0f);
	//ed::PushStyleVar(ed::StyleVar_PinArrowWidth, 0.0f);
	//ed::PushStyleVar(ed::StyleVar_TargetDirection, ImVec2(0.0f, 0.0f));
	//ed::BeginPin(1, ed::PinKind::Output);
	//ImGui::Text("Out");
	//ed::EndPin();
	//ed::EndNode();
	//ed::End();

	//ed::SetCurrentEditor(nullptr);

	ImGui::End();
}