#ifndef __PANEL_RESOURCEPREVIEW_H__
#define __PANEL_RESOURCEPREVIEW_H__

#include "Panel.h"

class Viewport;

class PanelResourcePreview : public Panel
{
public:
	PanelResourcePreview();
	~PanelResourcePreview();

	virtual void Draw() override;
	void SetResource(unsigned uid);
	void ReleaseResource();

private:
	unsigned binded_uid = 0;
	Viewport* preview = nullptr;
};

#endif // !__PANEL_RESOURCEPREVIEW_H__

