#include "MouseController.h"

#define OEMRESOURCE
#include <windows.h>
#include <assert.h>

MouseController::MouseController()
{
}

MouseController::~MouseController()
{
}

void MouseController::ChangeWindowsCursorIcon()
{
	ChangeCursorIcon("aero_arrow.cur", true);
}

void MouseController::ChangeCursorIcon(std::string cursor, bool windowsCursor)
{
#ifdef GAME_BUILD
	std::string cursorPath = !windowsCursor ? ".\\Resources\\Cursors\\" : "C:\\Windows\\Cursors\\";
	HCURSOR handCursor = LoadCursorFromFile(cursorPath.append(cursor).c_str());
	BOOL ret = SetSystemCursor(handCursor, OCR_NORMAL);
	assert(ret);
	DestroyCursor(handCursor);
#endif
}

