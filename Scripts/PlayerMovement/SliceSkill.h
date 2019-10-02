#ifndef  __SliceSkill_h__
#define  __SliceSkill_h__

#include "MeleeSkill.h"

class SliceSkill :
	public MeleeSkill
{
public:
	SliceSkill(PlayerMovement* PM, const char* trigger, ComponentBoxTrigger* attackBox);
	~SliceSkill();

	void Start() override;
	//void Update() override;

protected:
	void UseSkill() override;
};

#endif