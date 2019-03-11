#ifndef __PanelScene_h__
#define __PanelScene_h__

#include "Panel.h"
class PanelView :
	public Panel
{
public:
	PanelView();
	~PanelView();

	void Draw();

	void CreateFrameBuffer(int width, int height);

	void CreateMSAABuffers(int width, int height);

public:
	unsigned FBO = 0;
	unsigned RBO = 0;

	unsigned MSAAFBO = 0;
	unsigned MSAADEPTH = 0;
	unsigned MSAACOLOR = 0;
	int current_width = 0;
	int current_height = 0;

private:

	unsigned texture = 0; //Handled allocation inside class
};

#endif //__PanelScene_h__

