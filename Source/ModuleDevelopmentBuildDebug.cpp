#include "Application.h"
#include "ModuleDevelopmentBuildDebug.h"
#include "ModuleWindow.h"
#include "ModuleRender.h"
#include "ModuleScene.h"
#include "ModuleInput.h"
#include "GameObject.h"
#include "ComponentCamera.h"
#include "ComponentTransform.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"

// Called before render is available
bool ModuleDevelopmentBuildDebug::Init(JSON * config)
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

update_status ModuleDevelopmentBuildDebug::PreUpdate()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(App->window->window);
	ImGui::NewFrame();	
	return UPDATE_CONTINUE;
}

// Called every draw update
update_status ModuleDevelopmentBuildDebug::Update(float dt)
{
	ImGui::Begin("Neta - Vertical Slice 1 - Development build debug");
	++frames;
	float avgTime = (totalTime / (float)frames);
	float fps = 1 / avgTime;	
	totalTime += dt;
	ImGui::Text("Performance");
	ImGui::Separator();
	ImGui::Text("Average fps %.3f | Average frame time %.3f", fps, avgTime);
	if (frames > 1000)
	{
		minFps = MIN(minFps, fps);
		maxFps = MAX(maxFps, fps);
		minTime = MIN(minTime, dt);
		maxTime = MAX(maxTime, dt);
		ImGui::Text("Min fps %.3f | Max fps %.3f", minFps, maxFps);
		ImGui::Text("Min time %.3f | Max time %.3f", minTime, maxTime);
	}
	ImGui::Separator();
	ImGui::Separator();
	ImGui::Text("Camera selector");

	if (ImGui::Button("Player Camera"))
	{
		GameObject* playerCam = App->scene->FindGameObjectByName("PlayerCamera");
		App->scene->maincamera->isMainCamera = false;
		ComponentCamera* cam = (ComponentCamera*) playerCam->GetComponent(ComponentType::Camera);
		cam->isMainCamera = true;
		App->scene->maincamera = cam;
	}
	if (ImGui::Button("God Mode Camera"))
	{
		GameObject* godModeCam = App->scene->FindGameObjectByName("GodModeCamera");		
		App->scene->maincamera->isMainCamera = false;
		godModeCamera = (ComponentCamera*)godModeCam->GetComponent(ComponentType::Camera);
		godModeCamera->SetAspect((float)App->window->width / (float)App->window->height);
		godModeCamera->isMainCamera = true;
		App->scene->maincamera = godModeCamera;
		godModeCamera->gameobject = nullptr;
	}
	if (ImGui::Button("Static Camera 1"))
	{
		GameObject* staticModeCam = App->scene->FindGameObjectByName("StaticCamera1");
		App->scene->maincamera->isMainCamera = false;
		ComponentCamera* cam = (ComponentCamera*)staticModeCam->GetComponent(ComponentType::Camera);
		cam->SetAspect((float)App->window->width / (float)App->window->height);
		cam->isMainCamera = true;
		App->scene->maincamera = cam;
	}
	if (ImGui::Button("Static Camera 2"))
	{
		GameObject* staticModeCam = App->scene->FindGameObjectByName("StaticCamera2");
		App->scene->maincamera->isMainCamera = false;
		ComponentCamera* cam = (ComponentCamera*)staticModeCam->GetComponent(ComponentType::Camera);
		cam->SetAspect((float)App->window->width / (float)App->window->height);
		cam->isMainCamera = true;
		App->scene->maincamera = cam;
	}
	

	if (godModeCamera == App->scene->maincamera)
	{
		if (App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_REPEAT)
		{
			float mouse_motion_x, mouse_motion_y;
			App->input->SetMouseMotion(mouse_motion_x, mouse_motion_y);

			float dx = godModeCamera->rotationSpeed *  mouse_motion_x;
			float dy = godModeCamera->rotationSpeed *  mouse_motion_y;
			godModeCamera->Rotate(dx, dy);

			float3 movement = float3::zero;

			if (App->input->IsKeyPressed(SDL_SCANCODE_Q))
			{
				movement += float3::unitY;
			}
			if (App->input->IsKeyPressed(SDL_SCANCODE_E))
			{
				movement -= float3::unitY;
			}
			if (App->input->IsKeyPressed(SDL_SCANCODE_DOWN))
			{
				movement -= godModeCamera->frustum->front;
			}
			if (App->input->IsKeyPressed(SDL_SCANCODE_UP))
			{
				movement += godModeCamera->frustum->front;
			}
			if (App->input->IsKeyPressed(SDL_SCANCODE_LEFT))
			{
				movement -= godModeCamera->frustum->WorldRight();
			}
			if (App->input->IsKeyPressed(SDL_SCANCODE_RIGHT))
			{
				movement += godModeCamera->frustum->WorldRight();
			}
			if (App->input->IsKeyPressed(SDL_SCANCODE_LSHIFT))
			{
				movement *= 2;
			}
			godModeCamera->Move(movement*dt*App->renderer->current_scale);

		}
	}
	ImGui::End();
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleDevelopmentBuildDebug::CleanUp()
{
	LOG("Destroying editor");

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	return true;
}

void ModuleDevelopmentBuildDebug::RenderGUI() const
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ModuleDevelopmentBuildDebug::processInput(SDL_Event * event) const
{
	assert(event != NULL);
	ImGui_ImplSDL2_ProcessEvent(event);
}
