#ifndef __PanelConsole_h__
#define __PanelConsole_h__

#include "Panel.h"
#include <vector>
struct ImGuiTextBuffer;

class PanelConsole :
	public Panel
{
public:
	PanelConsole();
	~PanelConsole();

	void Clear();
	void AddLog(const char * log);
	void Draw();

private:
	bool forceScrollToBottom = true;
	ImGuiTextBuffer* buf = nullptr;
	std::vector<int> lineOffsets;        // Index to lines offset
	bool scrollToBottom = false;
};

#endif //__PanelConsole_h__
