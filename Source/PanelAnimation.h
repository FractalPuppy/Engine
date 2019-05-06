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
	std::vector<std::string> guiAnimations;

	ResourceAnimation* anim = nullptr;
	//bool isPlaying = false;

};

#endif // !__PanelAnimation_h__