#include "Application.h"

#include "ModuleCamera.h"
#include "ModuleInput.h"
#include "ModuleWindow.h"
#include "ModuleEditor.h"
#include "ModuleRender.h"

#include "ComponentCamera.h"
#include "PanelHierarchy.h"

#include "JSON.h"
#include "imgui.h"
#include "Math/MathFunc.h"
#include "Geometry/Frustum.h"
#include "Brofiler.h"

ModuleCamera::ModuleCamera()
{
}

ModuleCamera::~ModuleCamera()
{
	RELEASE(editorcamera);
}

bool ModuleCamera::Init(JSON* config)
{
	editorcamera = new ComponentCamera();

	JSON_value* cam = config->GetValue("camera");
	if (cam == nullptr) return true;

	Frustum *frustum = editorcamera->frustum;
	frustum->pos = cam->GetFloat3("position");
	frustum->front = cam->GetFloat3("forward");
	frustum->up = cam->GetFloat3("up");

	editorcamera->movementSpeed = cam->GetFloat("movement");
	editorcamera->rotationSpeed = cam->GetFloat("rotation");
	editorcamera->zoomSpeed = cam->GetFloat("zoom");
	editorcamera->SetFOV(cam->GetFloat("fov"));
	frustum->nearPlaneDistance = cam->GetFloat("znear");
	
	return true;
}

void ModuleCamera::SaveConfig(JSON * config)
{
	JSON_value* cam = config->CreateValue();

	Frustum *frustum = editorcamera->frustum;
	cam->AddFloat3("position", frustum->pos);
	cam->AddFloat3("forward", frustum->front);
	cam->AddFloat3("up", frustum->up);
	cam->AddFloat("movement", editorcamera->movementSpeed);
	cam->AddFloat("rotation", editorcamera->rotationSpeed);
	cam->AddFloat("zoom", editorcamera->zoomSpeed);
	cam->AddFloat("fov", frustum->verticalFov);
	cam->AddFloat("znear", frustum->nearPlaneDistance);
	cam->AddFloat("zfar", frustum->farPlaneDistance);

	config->AddValue("camera", *cam);
}

update_status ModuleCamera::Update(float dt)
{
	BROFILER_CATEGORY("Camera Update", Profiler::Color::Red)
	
	if (App->renderer->IsSceneViewFocused() || App->editor->hierarchy->IsHovered())
	{
		if(App->renderer->IsSceneHovered())
		{
		InputMove(dt);
		InputRotate(dt);
		InputCenter();
		InputOrbit(dt);
		}
	}
	if (App->renderer->IsSceneHovered())
	{
		InputZoom();
	}

	return UPDATE_CONTINUE;
}

bool ModuleCamera::CleanUp()
{
	return editorcamera->CleanUp();
}

void ModuleCamera::InputMove(float dt) const
{
	if (App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_REPEAT)
	{
		float3 movement = float3::zero;

		if (App->input->IsKeyPressed(SDL_SCANCODE_Q))
		{
			movement += float3::unitY;
		}
		if (App->input->IsKeyPressed(SDL_SCANCODE_E))
		{
			movement -= float3::unitY;
		}
		if (App->input->IsKeyPressed(SDL_SCANCODE_S))
		{
			movement -= editorcamera->frustum->front;
		}
		if (App->input->IsKeyPressed(SDL_SCANCODE_W))
		{
			movement += editorcamera->frustum->front;
		}
		if (App->input->IsKeyPressed(SDL_SCANCODE_A))
		{
			movement -= editorcamera->frustum->WorldRight();
		}
		if (App->input->IsKeyPressed(SDL_SCANCODE_D))
		{
			movement += editorcamera->frustum->WorldRight();
		}
		if (App->input->IsKeyPressed(SDL_SCANCODE_LSHIFT))
		{
			movement *= 2;
		}
		editorcamera->Move(movement*dt*App->renderer->current_scale);
	}
	else if (App->input->GetMouseButtonDown(SDL_BUTTON_MIDDLE) == KEY_REPEAT)
	{
		math::float3 motion = { App->input->GetMouseMotion().x, App->input->GetMouseMotion().y, 0.0f };
		math::float3 movement = (-motion.x * editorcamera->frustum->WorldRight() * mouseSens) + (motion.y * editorcamera->frustum->up * mouseSens);

		editorcamera->Move(movement*dt*App->renderer->current_scale);
	}
}

void ModuleCamera::InputRotate(float dt) const
{
	if (App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_REPEAT)
	{
		float mouse_motion_x, mouse_motion_y;
		App->input->SetMouseMotion(mouse_motion_x, mouse_motion_y);

		float dx = editorcamera->rotationSpeed * mouse_motion_x;
		float dy = editorcamera->rotationSpeed * mouse_motion_y;
		editorcamera->Rotate(dx,dy);
	}
}

void ModuleCamera::InputCenter() const
{
	if (App->input->GetKey(SDL_SCANCODE_F) == KEY_DOWN)
	{
		editorcamera->Center();
	}
}

void ModuleCamera::InputOrbit(float dt) const
{
	if (App->input->IsKeyPressed(SDL_SCANCODE_LALT) && App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT)
	{
		float mouse_motion_x, mouse_motion_y;
		App->input->SetMouseMotion(mouse_motion_x, mouse_motion_y);

		editorcamera->Orbit(editorcamera->rotationSpeed * mouse_motion_x,
		editorcamera->rotationSpeed *  mouse_motion_y);
	}
}

void ModuleCamera::InputZoom() const
{
	editorcamera->Zoom(App->input->GetMouseWheel(), App->input->IsKeyPressed(SDL_SCANCODE_LSHIFT));
}

void ModuleCamera::DrawGUI()
{
	Frustum *frustum = editorcamera->frustum;
	ImGui::InputFloat3("Position", (float*)&frustum->pos, 2, ImGuiInputTextFlags_ReadOnly);
	ImGui::InputFloat3("Forward ", (float*)&frustum->front, 2, ImGuiInputTextFlags_ReadOnly);

	ImGui::InputFloat("Movement Speed", &editorcamera->movementSpeed, 1.f, 5.f);
	ImGui::InputFloat("Rotation Speed", &editorcamera->rotationSpeed, 1.f, 5.f);
	ImGui::InputFloat("Zoom Speed", &editorcamera->zoomSpeed, 1.f, 5.f);

	float degFov = math::RadToDeg(frustum->verticalFov);
	if (ImGui::SliderFloat("FOV", &degFov, 40, 120))
	{
		frustum->verticalFov = math::DegToRad(degFov);
		frustum->horizontalFov = 2.f * atanf(tanf(frustum->verticalFov*0.5f)*App->window->width / App->window->height);
	}
	ImGui::InputFloat("Znear", &frustum->nearPlaneDistance, 1, 10);
	ImGui::InputFloat("Zfar", &frustum->farPlaneDistance, 1, 10);

	if(ImGui::Button("Reset Camera"))
	{
		editorcamera->ResetFrustum();
	}
}
