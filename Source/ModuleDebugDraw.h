#ifndef __Module_Debugdraw_h__
#define __Module_Debugdraw_h__

#include "Module.h"

class DDRenderInterfaceCoreGL;
class ComponentCamera;

class ModuleDebugDraw : public Module
{
	public:

		ModuleDebugDraw();
		~ModuleDebugDraw();

		bool            Init(JSON *json) override;
		bool            CleanUp() override;

		void            Draw(const ComponentCamera &camera, unsigned fb_width, unsigned fb_height);

	private:
		static DDRenderInterfaceCoreGL *implementation;
};

#endif /* __Module_Debugdraw_h__ */
