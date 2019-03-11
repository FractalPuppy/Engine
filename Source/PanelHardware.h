#ifndef __PanelHardware_h__
#define __PanelHardware_h__

#include "Panel.h"

class PanelHardware :
	public Panel
{
public:
	PanelHardware();
	~PanelHardware();

	void Draw();
};

#endif //__PanelHardware_h__