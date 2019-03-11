#include "PanelConsole.h"
#include "imgui.h"
#include "Globals.h"

PanelConsole::PanelConsole()
{
	buf = new ImGuiTextBuffer();
}


PanelConsole::~PanelConsole()
{
	RELEASE(buf);
}

void PanelConsole::Clear()
{
	buf->clear(); 
	lineOffsets.clear();
}

void PanelConsole::AddLog(const char * log)
{
	buf->appendf(log);
	scrollToBottom = true;
}

void PanelConsole::Draw()
{
	if (!ImGui::Begin("Log", &enabled))
	{
		ImGui::End();
		return;
	}
	if (ImGui::IsWindowHovered())
	{
		ImGui::SetWindowFocus();
	}
	if (ImGui::Button("Clear")) Clear();
	ImGui::SameLine();
	bool copy = ImGui::Button("Copy");
	ImGui::SameLine();
	ImGui::Separator();
	ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
	if (copy) ImGui::LogToClipboard();

	ImGui::TextUnformatted(buf->begin());

	if (scrollToBottom)
	{
		ImGui::SetScrollHereY(1.0f);
	}

	scrollToBottom = false;
	if (forceScrollToBottom)
	{
		scrollToBottom = true;
		forceScrollToBottom = false;
	}
	ImGui::EndChild();
	ImGui::End();
}
