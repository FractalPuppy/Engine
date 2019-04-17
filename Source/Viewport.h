#ifndef __Viewport_h__
#define __Viewport_h__

#include "Math/float3.h"
#include "Math/float2.h"
class ComponentCamera;

class Viewport
{
public:
	Viewport(std::string name);
	~Viewport();

	void Draw(ComponentCamera* cam, bool isEditor = false);

	void DrawImGuizmo(const ComponentCamera & cam);

	void Pick();

	void ToggleEnabled();
	inline bool IsEnabled() 
	{ 
		return enabled; 
	}

private:
	void DrawGuizmoButtons();
	void CreateFrameBuffer(int width, int height);
	void CreateMSAABuffers(int width, int height);

public:
	int current_width = 0;
	int current_height = 0;

	unsigned texture = 0; //Handled allocation inside class
	unsigned FBO = 0;
	unsigned RBO = 0;

	unsigned MSAAFBO = 0;
	unsigned MSAADEPTH = 0;
	unsigned MSAACOLOR = 0;
	bool focus = true;
	bool hover = false;
	math::float2 winPos = float2::zero;
private:
	bool enabled = true;
	bool startImguizmoUse = false;

	std::string name = "Viewport";

	bool useSnap = false; //TODO: save in config
	math::float3 snapSettings = math::float3::one;

	unsigned mCurrentGizmoOperation = 0; //Translate //We avoid imguizmo .h inclusion
	unsigned mCurrentGizmoMode = 1;//World
	unsigned mCurrentModeAux = 1;
};

#endif __Viewport_h__