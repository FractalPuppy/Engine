#ifndef __LOOPSTATECREDITS_H_
#define __LOOPSTATECREDITS_H_

#include "LoopState.h"

class CreditsScript;
class Button;

class LoopStateCredits :
	public LoopState
{
public:
	LoopStateCredits(GameLoop* GL);
	virtual ~LoopStateCredits();

	void Enter() override;
	void Update() override;

private:
	GameObject* backButtonGO = nullptr;
	Button* backButton = nullptr;
	float videoDuration = 0.0f;
	float videoTimer = 0.0f;
	bool started = false;
};

#endif // __LOOPSTATECREDITS_H_