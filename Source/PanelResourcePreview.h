#ifndef __PANEL_RESOURCEPREVIEW_H__
#define __PANEL_RESOURCEPREVIEW_H__

#include "Panel.h"

class Viewport;
class Resource;
class GameObject;
class ComponentCamera;

class PanelResourcePreview : public Panel
{
public:
	PanelResourcePreview();
	~PanelResourcePreview();

	void Init();
	virtual void Draw() override;

	void SetResource(unsigned uid);
	void SetResource(const char* name);
	void ReleaseResource();

private:
	void Render();

	void DisplayTexture();
	void DisplayMesh();


private:
	Viewport* preview = nullptr;
	Resource* resource = nullptr;

	GameObject* target = nullptr;
	GameObject* gameobject_camera = nullptr;
	ComponentCamera* camera = nullptr;
};

#endif // !__PANEL_RESOURCEPREVIEW_H__

