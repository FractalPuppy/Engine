#include "Application.h"

#include "ModuleScene.h"
#include "ComponentAnimation.h"

#include "GameObject.h"
#include "PanelState.h"
#include "ResourceStateMachine.h"

#include "HashString.h"
#include "NodeEditor.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#define DEFAULT_BLEND 300

PanelState::PanelState()
{
}

PanelState::~PanelState()
{

}

void PanelState::DrawSM(ResourceStateMachine * stateMachine, ax::NodeEditor::EditorContext * context)
{
	auto& io = ImGui::GetIO();

	ed::SetCurrentEditor(context);
	ed::Begin("State Machine Editor", ImVec2(0.0, 0.0f));

	DrawNodes(stateMachine);
	DrawTransitions(stateMachine);
	ManageCreate(stateMachine);
	ShowContextMenus(stateMachine);


	ed::Suspend();
	ShowNodeMenu(stateMachine);
	ShowTransitionMenu(stateMachine);
	ShowCreateNewNodeMenu(stateMachine);
	ed::Resume();

	ed::End();
	ed::SetCurrentEditor(nullptr);

}

void PanelState::Draw()
{
	if (!ImGui::Begin("State Machine", &enabled))
	{
		ImGui::End();
		return;
	}

	if (App->scene->selected != nullptr && (ComponentAnimation*)(App->scene->selected->GetComponentOld(ComponentType::Animation)))
	{
		ComponentAnimation* compAnim = (ComponentAnimation*)(App->scene->selected->GetComponentOld(ComponentType::Animation));
		if (compAnim->stateMachine != nullptr)
		{
			DrawSM(compAnim->stateMachine, compAnim->GetEditorContext());
		}
	}

	ImGui::End();
}

void PanelState::DrawNodes(ResourceStateMachine* stateMachine)
{
	unsigned nodesSize = stateMachine->GetNodesSize();
	for (unsigned i = 0u; i < nodesSize; ++i)
	{
		ed::PushStyleColor(ed::StyleColor_PinRect, ImColor(60, 180, 255, 150));
		ed::PushStyleColor(ed::StyleColor_PinRectBorder, ImColor(60, 180, 255, 150));

		ed::BeginNode(i*3 +1);
		ImGui::Indent(1.0);
		ImGui::Text(stateMachine->GetNodeName(i).C_str());
		if (i == stateMachine->GetDefaultNode())
		{
			ImGui::SameLine(); ImGui::Text("default");
		}

		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f * ImGui::GetStyle().Alpha);

		ImVec2 size = ed::GetNodeSize(i * 3 + 1);
		ImVec2 pos = ed::GetNodePosition(i * 3 + 2);

		ImDrawList* drawList = ed::GetNodeBackgroundDrawList(i * 3 + 1);

		drawList->AddLine(
			ImGui::GetCursorScreenPos(),
			ImGui::GetCursorScreenPos() + ImVec2(size.x - 16.0f, 0.0),
			IM_COL32(255, 255, 0, 255), 1.0f);


		ImGui::PopStyleVar();

		ImGui::Dummy(ImVec2(96.0, 8.0));

		ImGui::Text("clip: %s", stateMachine->GetNodeClip(i).C_str());

		drawList->AddLine(
			ImGui::GetCursorScreenPos(),
			ImGui::GetCursorScreenPos() + ImVec2(size.x - 16.0f, 0.0),
			IM_COL32(255, 255, 255, 255), 1.0f);

		ImGui::Dummy(ImVec2(64.0, 8.0));

		// In Pin
		ed::PushStyleVar(ed::StyleVar_PinArrowSize, 8.0f);
		ed::PushStyleVar(ed::StyleVar_PinArrowWidth, 8.0f);
		ed::PushStyleVar(ed::StyleVar_PinRadius, 10.0f);
		ed::PushStyleVar(ed::StyleVar_TargetDirection, ImVec2(0.0f, 0.0f));
		ed::BeginPin(i * 3 + 2, ed::PinKind::Input);
		ImGui::Text("In");
		ed::EndPin();
		ed::PopStyleVar(4);

		// Out Pin
		ImGui::SameLine(size.x - 40);
		ed::PushStyleVar(ed::StyleVar_PinArrowSize, 0.0f);
		ed::PushStyleVar(ed::StyleVar_PinArrowWidth, 0.0f);
		ed::PushStyleVar(ed::StyleVar_TargetDirection, ImVec2(0.0f, 0.0f));
		ed::BeginPin(i * 3 + 3, ed::PinKind::Output);
		ImGui::Text("Out");
		ed::EndPin();


		ed::EndNode();

		ed::PopStyleVar(3);
		ed::PopStyleColor(2);


	}
}

void PanelState::DrawTransitions(ResourceStateMachine * stateMachine)
{
	ed::PushStyleVar(ed::StyleVar_LinkStrength, 4.0f);
	unsigned numNodes = stateMachine->GetNodesSize();
	for (unsigned i = 0u, count = stateMachine->GetTransitionsSize(); i < count; ++i)
	{
		unsigned origin = stateMachine->FindNode(stateMachine->GetTransitionOrigin(i));
		unsigned destiny = stateMachine->FindNode(stateMachine->GetTransitionDestiny(i));

		if (origin < numNodes && destiny < numNodes)
		{
			ed::Link(numNodes * 3 + i + 1, origin * 3 + 3, destiny * 3 + 2);
		}
	}
	ed::PopStyleVar(1);

}

void PanelState::ShowContextMenus(ResourceStateMachine* stateMachine)
{
	ed::Suspend();

	ed::NodeId contextNodeId = 0;
	ed::PinId contextPinId = 0;
	ed::LinkId contextLinkId = 0;
	if (ed::ShowNodeContextMenu(&contextNodeId))
	{
		contextNode = unsigned(contextNodeId.Get() - 1) / 3;
		ImGui::OpenPopup("Node Context Menu");
	}
	else if (ed::ShowPinContextMenu(&contextPinId))
	{
		ImGui::OpenPopup("Pin Context Menu");
	}
	else if (ed::ShowLinkContextMenu(&contextLinkId))
	{
		contextLink = unsigned(contextLinkId.Get()) - stateMachine->GetNodesSize() * 3 - 1;
		ImGui::OpenPopup("Link Context Menu");
	}
	else if (ed::ShowBackgroundContextMenu())
	{
		newNodePosition = ImGui::GetMousePos();
		newNodePin = 0;
		ImGui::OpenPopup("Create New Node");
	}

	ed::Resume();
}

void PanelState::ShowNodeMenu(ResourceStateMachine * stateMachine)
{
	if (ImGui::BeginPopup("Node Context Menu"))
	{
		ImGui::TextUnformatted("Transition Context Menu");
		ImGui::Separator();

		//Node Name
		char* nodeName = new char[MAX_CLIP_NAME];
		strcpy(nodeName, stateMachine->GetNodeName(contextNode).C_str());
		ImGui::InputText("Node name", nodeName, MAX_CLIP_NAME);
		//WARNING, if the node name changes the transitions containing it must also change!
		stateMachine->RenameTransitionDueNodeChanged(stateMachine->GetNodeName(contextNode), HashString(nodeName));
		stateMachine->SetNodeName(contextNode, HashString(nodeName));

		//We need a checkbox to determine the defaultNode
		bool isDefault = stateMachine->GetDefaultNode() == contextNode;
		if (ImGui::Checkbox("DefaultNode", &isDefault))
		{
			//Set the context node to be the default
			stateMachine->SetDefaultNode(contextNode);
			stateMachine->Save();
		}

		if(ImGui::BeginCombo("Clip", stateMachine->GetNodeClip(contextNode).C_str()))
		{
			for (unsigned i = 0u; i < stateMachine->GetClipsSize(); i++)
			{
				bool isSelected = stateMachine->GetNodeClip(contextNode) == stateMachine->GetClipName(i);
				if (ImGui::Selectable(stateMachine->GetClipName(i).C_str(), isSelected))
				{
					stateMachine->SetNodeClip(contextNode, stateMachine->GetClipName(i));
					stateMachine->Save();
				}
				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Delete"))
		{
			ed::DeleteNode(ed::NodeId((contextNode + 1) * 3));
			stateMachine->RemoveNode(contextNode);
			stateMachine->Save();
		}
		ImGui::EndPopup();
	}
}

void PanelState::ShowTransitionMenu(ResourceStateMachine* stateMachine)
{
	if (ImGui::BeginPopup("Link Context Menu"))
	{
		ImGui::TextUnformatted("Link Context Menu");
		ImGui::Separator();

		//Origin
		char* oName = new char[MAX_CLIP_NAME];
		strcpy(oName, stateMachine->GetTransitionOrigin(contextLink).C_str());
		ImGui::InputText("Origin", oName, MAX_CLIP_NAME);
		stateMachine->SetTransitionOrigin(contextLink, HashString(oName));

		//Destiny
		char* dName = new char[MAX_CLIP_NAME];
		strcpy(dName, stateMachine->GetTransitionDestiny(contextLink).C_str());
		ImGui::InputText("Destiny", dName, MAX_CLIP_NAME);
		stateMachine->SetTransitionDestiny(contextLink, HashString(dName));

		//Trigger
		char* tName = new char[MAX_CLIP_NAME];
		strcpy(tName, stateMachine->GetTransitionTrigger(contextLink).C_str());
		ImGui::InputText("Trigger", tName, MAX_CLIP_NAME);
		stateMachine->SetTransitionTrigger(contextLink, HashString(tName));

		//Blend
		float tBlend = stateMachine->GetTransitionBlend(contextLink);
		if (ImGui::InputFloat("Blend", &tBlend))
		{
			stateMachine->SetTransitionBlend(contextLink, tBlend);
		}

		//delete
		if (ImGui::MenuItem("Delete"))
		{
			ed::DeleteLink(ed::LinkId((contextLink + 1) + 2));
			stateMachine->RemoveTransition(contextLink);
			stateMachine->Save();
		}

		ImGui::EndPopup();
	}
}

void PanelState::ShowCreateNewNodeMenu(ResourceStateMachine* stateMachine)
{
	if (ImGui::BeginPopup("Create New Node"))
	{
		ImGui::TextUnformatted("Create Node Menu");
		ImGui::Separator();

		if (ImGui::BeginMenu("New animation"))
		{
			for (unsigned i = 0, count = stateMachine->GetClipsSize(); i < count; ++i)
			{
				if (ImGui::MenuItem(stateMachine->GetClipName(i).C_str()))
				{
					unsigned nodeIndex = stateMachine->GetNodesSize();
					ed::SetNodePosition(nodeIndex * 3 + 1, ed::ScreenToCanvas(newNodePosition));
					AddNode(stateMachine, HashString("Node A") ,stateMachine->GetClipName(i));		
					stateMachine->Save();
					if (newNodePin != ed::PinId::Invalid)
					{
						unsigned outNode = unsigned(newNodePin.Get() - 1) / 3;
						stateMachine->AddTransition(stateMachine->GetNodeName(outNode),
							stateMachine->GetNodeName(nodeIndex), HashString(""), DEFAULT_BLEND);
						stateMachine->Save();
					}
				}
			}
			ImGui::EndMenu();
		}
		ImGui::EndPopup();
	}
}

void PanelState::AddNode(ResourceStateMachine* stateMachine, HashString nodeName, HashString clipName)
{
	for (unsigned i = 0u; i < stateMachine->GetNodesSize(); i++)
	{
		if (stateMachine->GetNodeName(i) == nodeName)
		{
			nodeName = HashString("Carefull, always rename after creation!");
		}
	}
	stateMachine->AddNode(nodeName, clipName);
}

void PanelState::ManageCreate(ResourceStateMachine* stateMachine)
{
	if (ed::BeginCreate(ImColor(255, 255, 255), 2.0f))
	{
	auto showLabel = [](const char* label, ImColor color)
	{
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetTextLineHeight());
		auto size = ImGui::CalcTextSize(label);

		auto padding = ImGui::GetStyle().FramePadding;
		auto spacing = ImGui::GetStyle().ItemSpacing;

		ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(spacing.x, -spacing.y));

		auto rectMin = ImGui::GetCursorScreenPos() - padding;
		auto rectMax = ImGui::GetCursorScreenPos() + size + padding;

		auto drawList = ImGui::GetWindowDrawList();
		drawList->AddRectFilled(rectMin, rectMax, color, size.y * 0.15f);
		ImGui::TextUnformatted(label);
	};

	ed::PinId startPinId = 0, endPinId = 0;
	if (ed::QueryNewLink(&startPinId, &endPinId))
	{
		if (startPinId && endPinId)
		{
			bool startIsInput = unsigned(startPinId.Get() - 1) % 3 == 1;
			bool endIsInput = unsigned(endPinId.Get() - 1) % 3 == 1;
			unsigned startNode = unsigned(startPinId.Get() - 1) / 3;
			unsigned endNode = unsigned(endPinId.Get() - 1) / 3;

			if (endPinId == startPinId)
			{
				ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
			}
			else if (startIsInput == endIsInput)
			{
				showLabel("x Incompatible Pins. Must be In->Out", ImColor(45, 32, 32, 180));
				ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
			}
			else if (startNode == endNode)
			{
				showLabel("x Cannot connect to self", ImColor(45, 32, 32, 180));
				ed::RejectNewItem(ImColor(255, 0, 0), 1.0f);
			}
			else
			{
				showLabel("+ Create Link", ImColor(32, 45, 32, 180));
				if (ed::AcceptNewItem(ImColor(128, 255, 128), 4.0f))
				{
					if (startIsInput)
					{
						stateMachine->AddTransition(stateMachine->GetNodeName(endNode),
							stateMachine->GetNodeName(startNode), HashString(""), DEFAULT_BLEND);
					}
					else
					{
						stateMachine->AddTransition(stateMachine->GetNodeName(startNode),
							stateMachine->GetNodeName(endNode), HashString(""), DEFAULT_BLEND);
					}
					stateMachine->Save();
				}
			}
		}
	}

	if (ed::QueryNewNode(&newNodePin))
	{
		bool pinIsInput = unsigned(newNodePin.Get() - 1) % 3 == 1;

		if (!pinIsInput)
		{
			if (newNodePin != ed::PinId::Invalid)
			{
				showLabel("+ Create Node", ImColor(32, 45, 32, 180));
			}

			if (ed::AcceptNewItem())
			{
				ed::Suspend();
				newNodePosition = ImGui::GetMousePos();
				ImGui::OpenPopup("Create New Node");
				ed::Resume();
			}
		}
	}
	}

ed::EndCreate();
}