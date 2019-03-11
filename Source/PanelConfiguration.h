#ifndef __PanelConfiguration_h__
#define __PanelConfiguration_h__

#include "Panel.h"
#include <vector>

class PanelConfiguration :
	public Panel
{
public:
	PanelConfiguration();
	~PanelConfiguration();

	void Draw();
	void AddFps(float dt);

	void UpdateExtrems();

private:
	void DrawFPSgraph() const;
	void DrawMSgraph() const;
	void DrawMemoryStats() const;

private:
	std::vector<float> fps;
	std::vector<float> ms;
	float totalfps = 0.f;
	float minfps;

	float totalms = 0.f;
	float maxms;
};

#endif //__PanelConfiguration_h__