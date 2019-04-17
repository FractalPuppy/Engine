#ifndef __PanelAnimation_h__
#define __PanelAnimation_h__

#include "Panel.h"

class PanelAnimation : public Panel
{
public:
	PanelAnimation();
	~PanelAnimation();

	void Draw() override;

	void UpdateGameObjectAnimation(GameObject* go, ResourceAnimation* anim);
	void CreateAnimationFromClip(ResourceAnimation* anim, int minFrame, int maxFrame);

public:
	int minFrame;
	int maxFrame;
	bool isCliping = false;

	//bool isPlaying = false;

};

#endif // !__PanelAnimation_h__