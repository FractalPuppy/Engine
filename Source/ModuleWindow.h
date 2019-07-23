#ifndef __ModuleWindow_H__
#define __ModuleWindow_H__

#include "Module.h"
#include "Math/float2.h"

class JSON;
struct SDL_Window; 

class ModuleWindow : public Module
{
public:

	ModuleWindow();

	virtual ~ModuleWindow();


	bool Init(JSON * config) override;

	bool CleanUp() override;
	void SaveConfig(JSON* config) override;

	void Resize(int width, int height);
	math::float2 GetWindowSize() const;
	math::float2 GetWindowPos() const;

	void DrawGUI() override;

public:
	//The window we'll be rendering to
	SDL_Window* window = NULL;
	
	int width = SCREEN_WIDTH;
	int height = SCREEN_HEIGHT;

private:

	float brightness = 1.0f;
	bool fullscreen = false;
	bool resizable = true;
	bool borderless = false;
	bool fullscreen_desktop = false;

	int previouswidth = SCREEN_WIDTH;
	int previousheight = SCREEN_HEIGHT;

};

#endif // __ModuleWindow_H__