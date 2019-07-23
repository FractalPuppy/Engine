#include "Globals.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleRender.h"
#include "Viewport.h"

#include "imgui.h"
#include "SDL.h"
#include "JSON.h"

#include <sstream>

ModuleWindow::ModuleWindow()
{
}

// Destructor
ModuleWindow::~ModuleWindow()
{
}

// Called before render is available
bool ModuleWindow::Init(JSON * config)
{
	LOG("Init SDL window & surface");
	bool ret = true;

	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		LOG("SDL_VIDEO could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		JSON_value* windowConfig = config->GetValue("window");
#ifndef GAME_BUILD
		if (windowConfig != nullptr)
		{
			fullscreen = windowConfig->GetInt("fullscreen");
			resizable = windowConfig->GetInt("resizable");
			borderless = windowConfig->GetInt("borderless");
			fullscreen_desktop = windowConfig->GetInt("fullscreen_desktop");
			width = windowConfig->GetInt("width");
			height = windowConfig->GetInt("height");
			brightness = windowConfig->GetFloat("brightness");
		}
		else
		{
			int width = SCREEN_WIDTH;
			int height = SCREEN_HEIGHT;
		}
#else
		fullscreen = true;
#endif
		//Create window
		Uint32 flags = SDL_WINDOW_SHOWN |  SDL_WINDOW_OPENGL;

		if (fullscreen)
		{
			flags |= SDL_WINDOW_FULLSCREEN;
		}

		if (resizable)
		{
			flags |= SDL_WINDOW_RESIZABLE;
		}

		if (borderless)
		{
			flags |= SDL_WINDOW_BORDERLESS;
		}

		if (fullscreen_desktop)
		{
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		}

		std::stringstream titleVersion;
		titleVersion << TITLE << " " << VERSION_BUILD;
		window = SDL_CreateWindow(titleVersion.str().c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);

		if(window == NULL)
		{
			LOG("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			ret = false;
		}
		SDL_GetWindowSize(window, &width, &height);
		//SDL_SetWindowBrightness(window, brightness);
	}

	return ret;
}

// Called before quitting
bool ModuleWindow::CleanUp()
{
	LOG("Destroying SDL window and quitting all SDL systems");

	//Destroy window
	if(window != NULL)
	{
		SDL_DestroyWindow(window);
	}

	//Quit SDL subsystems
	SDL_Quit();
	return true;
}

void ModuleWindow::SaveConfig(JSON * config)
{
	JSON_value* windowConfig = config->CreateValue();

	windowConfig->AddInt("fullscreen", fullscreen);
	windowConfig->AddInt("resizable", resizable);
	windowConfig->AddInt("borderless", borderless);
	windowConfig->AddInt("fullscreen_desktop", fullscreen_desktop);
	windowConfig->AddInt("width", width);
	windowConfig->AddInt("height", height);
	windowConfig->AddFloat("brightness", brightness);

	config->AddValue("window", *windowConfig);
}


void ModuleWindow::Resize(int width, int height)
{
	this->width = width;
	this->height = height;
	App->renderer->OnResize();
}

math::float2 ModuleWindow::GetWindowSize() const
{
#ifdef GAME_BUILD
	return math::float2(width, height);
#else
	Viewport* viewport = App->renderer->GetActiveViewport();
	return math::float2(viewport->current_width, viewport->current_height);
#endif // GAME_BUILD

}

math::float2 ModuleWindow::GetWindowPos() const
{
#ifdef GAME_BUILD
	return math::float2::zero;
#else
	return App->renderer->GetActiveViewport()->winPos;
#endif // GAME_BUILD
}

void ModuleWindow::DrawGUI()
{
	if ((!fullscreen || !fullscreen_desktop) && (ImGui::InputInt("width", &width,10,50) ||
		ImGui::InputInt("height", &height, 10, 50)))
	{
		if (width < 400) 
		{
			width = 400;
		}
		if(height < 400)
		{
			height = 400;
		}
		SDL_SetWindowSize(window, width, height);
		App->renderer->OnResize();
	}
	if (ImGui::SliderFloat("Brightness", &brightness, 0.0f, 1.0f))
	{
		SDL_SetWindowBrightness(window, brightness);
	}
	if (ImGui::Checkbox("FullScreen", &fullscreen))
	{
		if (fullscreen) {
			previousheight = height;
			previouswidth = width;

			SDL_DisplayMode displayMode;
			SDL_GetDesktopDisplayMode(0, &displayMode);
			SDL_SetWindowSize(window, displayMode.w, displayMode.h);
			SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
			Resize(displayMode.w, displayMode.h);
		}
		else
		{
			SDL_SetWindowFullscreen(window, 0);
			SDL_SetWindowSize(window, previouswidth, previousheight);
			Resize(previouswidth, previousheight);
			
		}
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("Resizable", &resizable))
	{
		if (resizable)
			SDL_SetWindowResizable(window, (SDL_bool)true);
		else
			SDL_SetWindowResizable(window, (SDL_bool)false);
	}

	ImGui::NewLine();
	if (ImGui::Checkbox("Borderless", &borderless))
	{
		if (borderless)
			SDL_SetWindowBordered(window, (SDL_bool)false);
		else
			SDL_SetWindowBordered(window, (SDL_bool)true);
	}

	ImGui::SameLine();
	if (ImGui::Checkbox("Full Desktop", &fullscreen_desktop))
	{
		if (fullscreen_desktop) {
			SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
		}
		else
			SDL_SetWindowFullscreen(window, 0);
	}
}
