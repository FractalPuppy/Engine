#ifndef __PanelInspector_h__
#define __PanelInspector_h__

#include "Panel.h"
class GameObject;

class PanelInspector :
	public Panel
{
public:
	PanelInspector();
	~PanelInspector();

	void Draw() override;
	void SetFocus();


	void DrawWarningPopup();

private:
	bool openPopup = false;
	std::string popUpSentence;
	std::vector<std::pair<const char*, ComponentType>> componentList;
};

#endif //__PanelInspector_h__

