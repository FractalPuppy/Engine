#ifndef __ModuleCamera_h__
#define __ModuleCamera_h__

#include "Module.h"
class ComponentCamera;
class JSON;

class ModuleCamera : public Module
{
public:
    ModuleCamera();
    ~ModuleCamera();

	bool Init(JSON * config) override;
	void SaveConfig(JSON * config) override;
	update_status Update(float dt) override;
	bool CleanUp() override;

	void InputMove(float dt) const;
	void InputRotate(float dt) const;
	void InputCenter() const;
	void InputOrbit(float dt) const;
	void InputZoom() const;

	void DrawGUI() override;

public:
	ComponentCamera * editorcamera = nullptr;

private:
	int mouseSens = 1000;
};

#endif /* __ModuleCamera_h__ */
