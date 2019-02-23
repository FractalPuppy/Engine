#ifndef __Module_h__
#define __Module_h__

#include "Globals.h"
class JSON;

class Module
{
public:

	Module()
	{
	}

	virtual ~Module() {};

	virtual bool Init(JSON *json) 
	{
		return true; 
	}

	virtual void SaveConfig(JSON *config) {};

	virtual void DrawGUI() {};

	virtual bool Start()
	{
		return true;
	}

	virtual update_status PreUpdate()
	{
		return UPDATE_CONTINUE;
	}

	virtual update_status Update(float dt)
	{
		return UPDATE_CONTINUE;
	}

	virtual update_status PostUpdate()
	{
		return UPDATE_CONTINUE;
	}

	virtual bool CleanUp() 
	{ 
		return true; 
	}
};

#endif __Module_h__