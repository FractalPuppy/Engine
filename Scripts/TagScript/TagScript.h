#ifndef  __TagScript_h__
#define  __TagScript_h__

#include "BaseScript.h"

#ifdef TagScript_EXPORTS
#define TagScript_API __declspec(dllexport)
#else
#define TagScript_API __declspec(dllimport)
#endif

class TagScript_API TagScript : public Script
{
	void Start() override;
	void Update() override;
};

#endif __TagScript_h__
