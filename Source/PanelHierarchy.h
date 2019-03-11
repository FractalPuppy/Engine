#ifndef __PanelHierarchy_h__
#define __PanelHierarchy_h__

#include "Panel.h"
class GameObject;

class PanelHierarchy :
	public Panel
{
public:
	PanelHierarchy();
	~PanelHierarchy();

	void Draw();

};

#endif __PanelHierarchy_h__
