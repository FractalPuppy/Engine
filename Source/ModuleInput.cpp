#include "Application.h"

#include "ModuleInput.h"
#include "ModuleWindow.h"
#include "ModuleTime.h"
#include "ModuleEditor.h"
#include "ModuleFileSystem.h"

#include "SDL.h"
#include "imgui.h"
#include "Brofiler.h"

#define MAX_KEYS 300

ModuleInput::ModuleInput() : Module(), mouse({ 0, 0 }), mouse_motion({ 0,0 })
{
	keyboard = new KeyState[MAX_KEYS];
	memset(keyboard, KEY_IDLE, sizeof(KeyState) * MAX_KEYS);
	memset(mouse_buttons, KEY_IDLE, sizeof(KeyState) * NUM_MOUSE_BUTTONS);
}

// Destructor
ModuleInput::~ModuleInput()
{
	RELEASE_ARRAY(keyboard);
}

// Called before render is available
bool ModuleInput::Init(JSON * config)
{
	LOG("Init SDL input event system");
	bool ret = true;
	SDL_Init(0);

	if (SDL_InitSubSystem(SDL_INIT_EVENTS) < 0)
	{
		LOG("SDL_EVENTS could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	return ret;
}

// Called each loop iteration
update_status ModuleInput::PreUpdate()
{
	BROFILER_CATEGORY("Input PreUpdate", Profiler::Color::Magenta);

	static SDL_Event event;

	mouse_motion = { 0, 0 };
	memset(windowEvents, false, WE_COUNT * sizeof(bool));
	mouse_wheel = 0;

	const Uint8* keys = SDL_GetKeyboardState(NULL);

	for (int i = 0; i < MAX_KEYS; ++i)
	{
		if (keys[i] == 1)
		{
			if (keyboard[i] == KEY_IDLE)
				keyboard[i] = KEY_DOWN;
			else
				keyboard[i] = KEY_REPEAT;
		}
		else
		{
			if (keyboard[i] == KEY_REPEAT || keyboard[i] == KEY_DOWN)
				keyboard[i] = KEY_UP;
			else
				keyboard[i] = KEY_IDLE;
		}
	}

	for (int i = 0; i < NUM_MOUSE_BUTTONS; ++i)
	{
		if (mouse_buttons[i] == KEY_DOWN)
			mouse_buttons[i] = KEY_REPEAT;

		if (mouse_buttons[i] == KEY_UP)
			mouse_buttons[i] = KEY_IDLE;
	}

	while (SDL_PollEvent(&event) != 0)
	{
		App->editor->processInput(&event);
		switch (event.type)
		{
		case SDL_QUIT:
			windowEvents[WE_QUIT] = true;
			break;

		case SDL_WINDOWEVENT:
			switch (event.window.event)
			{
			case SDL_WINDOWEVENT_RESIZED:
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				App->window->Resize(event.window.data1, event.window.data2);
				break;
			case SDL_WINDOWEVENT_HIDDEN:
			case SDL_WINDOWEVENT_MINIMIZED:
			case SDL_WINDOWEVENT_FOCUS_LOST:
				windowEvents[WE_HIDE] = true;
				break;

				//case SDL_WINDOWEVENT_ENTER:
			case SDL_WINDOWEVENT_SHOWN:
			case SDL_WINDOWEVENT_FOCUS_GAINED:
			case SDL_WINDOWEVENT_MAXIMIZED:
			case SDL_WINDOWEVENT_RESTORED:
				windowEvents[WE_SHOW] = true;
				break;
			}
			break;
		case SDL_MOUSEBUTTONDOWN:			
			mouse_buttons[event.button.button - 1] = KEY_DOWN;
			break;

		case SDL_MOUSEBUTTONUP:
			mouse_buttons[event.button.button - 1] = KEY_UP;
			break;

		case SDL_MOUSEMOTION:
			mouse_motion.x = (float)event.motion.xrel / (float) App->window->width;
			mouse_motion.y = (float)event.motion.yrel / (float)App->window->height;
			mouse.x = (float)event.motion.x;
			mouse.y = (float)event.motion.y;
			break;

		case SDL_MOUSEWHEEL:
			mouse_wheel = event.wheel.y;
			break;

		case SDL_DROPFILE:
			char* dropped_file = event.drop.file;
			DropFile(dropped_file);
			SDL_free(dropped_file);
			break;
		}
	}

	if (GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN && App->time->gameState != GameState::RUN)
	{
		return UPDATE_STOP;
	}	
	else if (GetWindowEvent(EventWindow::WE_QUIT) == true)
	{
		return UPDATE_STOP;
	}

	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleInput::CleanUp()
{
	LOG("Quitting SDL event subsystem");
	SDL_QuitSubSystem(SDL_INIT_EVENTS);
	return true;
}

// ---------
bool ModuleInput::GetWindowEvent(EventWindow ev) const
{
	return windowEvents[ev];
}

const fPoint& ModuleInput::GetMousePosition() const
{
	return mouse;
}

const void ModuleInput::SetMouseMotion(float &x, float &y) const
{
	x = mouse_motion.x;
	y = mouse_motion.y;
}

const fPoint & ModuleInput::GetMouseMotion() const
{
	return mouse_motion;
}

const float ModuleInput::GetMouseWheel() const
{
	return mouse_wheel;
}

void ModuleInput::DrawGUI()
{
	ImGui::Text("Mouse position:");
	ImGui::Text("X:%.2f | Y:%.2f", mouse.x, mouse.y);
}

void ModuleInput::DropFile(char* dropped_file) const //TODO: Popup when added to ASSETS
{
	assert(dropped_file != NULL);
	std::string filename(dropped_file);
	std::string path(dropped_file);

	std::size_t found = filename.find_last_of("\\/");
	if (std::string::npos != found)
	{
		filename.erase(0, found + 1);
		path.erase(found + 1, path.length());
	}
	App->fsystem->CopyFromOutsideFS(dropped_file, (ASSETS+filename).c_str());
}
