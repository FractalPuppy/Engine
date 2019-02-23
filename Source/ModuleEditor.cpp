#include "Application.h"

#include "ModuleEditor.h"
#include "ModuleRender.h"
#include "ModuleWindow.h"
#include "ModuleScene.h"
#include "ModuleFileSystem.h"
#include "ModuleTextures.h"
#include "ModuleProgram.h"

#include "Viewport.h"

#include "GameObject.h"
#include "PanelConsole.h"
#include "PanelConfiguration.h"
#include "PanelInspector.h"
#include "PanelAbout.h"
#include "PanelHardware.h"
#include "PanelHierarchy.h"
#include "PanelTime.h"

#include "MaterialEditor.h"
#include "FileExplorer.h"
#include "GUICreator.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"
#include "ImGuizmo.h"
#include "Brofiler.h"
#include <vector>

ModuleEditor::ModuleEditor()
{
	panels.push_back(console = new PanelConsole());
	panels.push_back(configuration = new PanelConfiguration());
	panels.push_back(inspector = new PanelInspector());
	panels.push_back(about = new PanelAbout());
	panels.push_back(hardware = new PanelHardware());
	panels.push_back(hierarchy = new PanelHierarchy());
	panels.push_back(time = new PanelTime());

	materialEditor = new MaterialEditor();
	fileExplorer = new FileExplorer();
}

// Destructor
ModuleEditor::~ModuleEditor()
{
	for (std::list<Panel*>::reverse_iterator it = panels.rbegin(); it != panels.rend(); ++it)
	{
		RELEASE(*it);
	}
	RELEASE(materialEditor);
	RELEASE(fileExplorer);
	console = nullptr;
	panels.clear();
}

// Called before render is available
bool ModuleEditor::Init(JSON * config)
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui_ImplSDL2_InitForOpenGL(App->window->window, App->renderer->context);
	ImGui_ImplOpenGL3_Init("#version 130");

	ImGuiStyle * style = &ImGui::GetStyle();

	style->WindowPadding = ImVec2(15, 15);
	style->WindowRounding = 5.0f;
	style->FramePadding = ImVec2(5, 5);
	style->FrameRounding = 4.0f;
	style->ItemSpacing = ImVec2(12, 8);
	style->ItemInnerSpacing = ImVec2(8, 6);
	style->IndentSpacing = 25.0f;
	style->ScrollbarSize = 15.0f;
	style->ScrollbarRounding = 9.0f;
	style->GrabMinSize = 5.0f;
	style->GrabRounding = 3.0f;

	style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
	style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
	style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
	style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_Column] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ColumnHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_ColumnActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
	style->Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);

	return true;
}

update_status ModuleEditor::PreUpdate()
{
	BROFILER_CATEGORY("Editor PreUpdate", Profiler::Color::Pink)

		ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(App->window->window);
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	CreateDockSpace();
	return UPDATE_CONTINUE;
}

// Called every draw update
update_status ModuleEditor::Update(float dt)
{
	BROFILER_CATEGORY("Editor Update", Profiler::Color::Blue)

	bool savepopup = false;
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New"))
			{
				App->scene->ClearScene();
			}
			if (ImGui::MenuItem("Load Scene"))
			{
				fileExplorer->currentOperation = MenuOperations::LOAD;
				fileExplorer->extensionToFilter = FILETYPE::SCENE;
				std::string scenePath = SCENES;
				scenePath = scenePath.substr(0, scenePath.size() - 1);
				fileExplorer->SetPath(*scenePath.c_str());
				sprintf_s(fileExplorer->title, "Load Scene");
				fileExplorer->openFileExplorer = true;
			}
			if (ImGui::MenuItem("Add Scene"))
			{
				fileExplorer->currentOperation = MenuOperations::ADD;
				fileExplorer->extensionToFilter = FILETYPE::SCENE;
				std::string scenePath = SCENES;
				scenePath = scenePath.substr(0, scenePath.size() - 1);
				fileExplorer->SetPath(*scenePath.c_str());
				sprintf_s(fileExplorer->title, "Add Scene");
				fileExplorer->openFileExplorer = true;
			}
			if (ImGui::MenuItem("Save"))
			{
				if (!App->scene->name.empty())
				{
					App->scene->SaveScene(*App->scene->root, *App->scene->name.c_str(), *App->scene->path.c_str());
				}
				else
				{
					fileExplorer->currentOperation = MenuOperations::SAVE;
					fileExplorer->extensionToFilter = FILETYPE::SCENE;
					std::string scenePath = SCENES;
					scenePath = scenePath.substr(0, scenePath.size() - 1);
					fileExplorer->SetPath(*scenePath.c_str());
					sprintf_s(fileExplorer->title, "Save Scene");
					fileExplorer->openFileExplorer = true;
				}
			}
			if (ImGui::MenuItem("Save As..."))
			{
				fileExplorer->currentOperation = MenuOperations::SAVE;
				fileExplorer->extensionToFilter = FILETYPE::SCENE;
				std::string scenePath = SCENES;
				scenePath = scenePath.substr(0, scenePath.size() - 1);
				fileExplorer->SetPath(*scenePath.c_str());
				sprintf_s(fileExplorer->title, "Save Scene");
				sprintf_s(fileExplorer->filename, App->scene->name.c_str());
				fileExplorer->openFileExplorer = true;
			}
			if (ImGui::MenuItem("Exit", "Esc"))
			{
				ImGui::EndMenu();
				ImGui::EndMainMenuBar();
				ImGui::End();
				return UPDATE_STOP;
			}
			ImGui::EndMenu();
		}
		GUICreator::CreateElements(App->scene->root);
		if (ImGui::MenuItem("New Material"))
		{
			materialEditor->open = true;
			materialEditor->isCreated = true;
		}
		fileExplorer->Draw();
		materialEditor->Draw();
		WindowsMenu();
		HelpMenu();
		ImGui::EndMainMenuBar();
	}
	DrawPanels();
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleEditor::CleanUp()
{
	LOG("Destroying editor");

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	return true;
}

void ModuleEditor::RenderGUI() const
{
	PROFILE;
	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ModuleEditor::CreateDockSpace() const
{
	ImGui::SetNextWindowPos({ 0,0 });
	ImGui::SetNextWindowSize({ (float)App->window->width, (float)App->window->height });
	ImGui::SetNextWindowBgAlpha(0.0f);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace", NULL, window_flags);
	ImGui::PopStyleVar(3);

	ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
}

void ModuleEditor::DrawPanels()
{
	PROFILE;
	for (std::list<Panel*>::iterator it = panels.begin(); it != panels.end(); ++it)
		if ((*it)->IsEnabled())
		{
			(*it)->Draw();
		}
}

void ModuleEditor::WindowsMenu()
{
	if (ImGui::BeginMenu("Windows"))
	{
		if (ImGui::MenuItem("Console", nullptr, console->IsEnabled()))
		{
			console->ToggleEnabled();
		}
		if (ImGui::MenuItem("Configuration", nullptr, configuration->IsEnabled()))
		{
			configuration->ToggleEnabled();
		}
		if (ImGui::MenuItem("Properties", nullptr, inspector->IsEnabled()))
		{
			inspector->ToggleEnabled();
		}
		if (ImGui::MenuItem("Hierarchy", nullptr, hierarchy->IsEnabled()))
		{
			hierarchy->ToggleEnabled();
		}
		if (ImGui::MenuItem("Scene", nullptr, App->renderer->viewScene->IsEnabled()))
		{
			App->renderer->viewScene->ToggleEnabled();
		}
		if (ImGui::MenuItem("Game Camera", nullptr, App->renderer->viewGame->IsEnabled()))
		{
			App->renderer->viewGame->ToggleEnabled();
    }
		if (ImGui::MenuItem("Time control", nullptr, time->IsEnabled()))
		{
			time->ToggleEnabled();
		}
		ImGui::EndMenu();
	}
}

void ModuleEditor::HelpMenu()
{
	if (ImGui::BeginMenu("Help"))
	{
		if (ImGui::MenuItem("About"))
		{
			about->SetEnabled();
		}
		if (ImGui::MenuItem("Hardware"))
		{
			hardware->SetEnabled();
		}
		ImGui::EndMenu();
	}
}

void ModuleEditor::ShowInspector()
{
	if (inspector != nullptr)
	{
		inspector->SetFocus();
	}
}

void ModuleEditor::AddFpsLog(float dt) const
{
	if (configuration != nullptr)
	{
		configuration->AddFps(dt);
	}
}

void ModuleEditor::AddLog(const char *log) const
{
	assert(log != NULL);
	if (console != nullptr)
	{
		console->AddLog(log);
	}
}
void ModuleEditor::processInput(SDL_Event * event) const
{
	assert(event != NULL);
	ImGui_ImplSDL2_ProcessEvent(event);
}
