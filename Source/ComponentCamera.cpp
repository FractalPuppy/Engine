#include "Application.h"

#include "ModuleScene.h"
#include "ModuleWindow.h"
#include "ModuleRender.h"

#include "GameObject.h"
#include "ComponentCamera.h"
#include "ComponentTransform.h"

#include "JSON.h"
#include "GL/glew.h"
#include "Math/MathFunc.h"
#include "Imgui/imgui.h"
#include "Imguizmo.h"
#include "Geometry/Frustum.h"

#define MAXFOV 120
#define MINFOV 40

ComponentCamera::ComponentCamera() : Component(nullptr, ComponentType::Camera)
{
	InitFrustum();
}

ComponentCamera::ComponentCamera(GameObject * gameobject) : Component(gameobject, ComponentType::Camera)
{
	InitFrustum();
	assert(gameobject != nullptr);
	if (gameobject == nullptr) return;
	frustum->pos = gameobject->GetBoundingBox().CenterPoint();
}

ComponentCamera::ComponentCamera(const ComponentCamera & component) : Component(component)
{
	frustum = new Frustum (*component.frustum);
	movementSpeed = component.movementSpeed;
	rotationSpeed = component.rotationSpeed;
	zoomSpeed = component.zoomSpeed;
}


ComponentCamera::~ComponentCamera()
{
	if (frustum != nullptr)
	{
		RELEASE(frustum);
	}

	if (isMainCamera)
	{
		App->scene->maincamera = nullptr;
	}

	if (frustumVAO != 0)
	{
		glDeleteVertexArrays(1, &frustumVAO);
	}
	if (frustumVBO != 0)
	{
		glDeleteBuffers(1, &frustumVBO);
	}
	if (frustumEBO != 0)
	{
		glDeleteBuffers(1, &frustumEBO);
	}
}

ComponentCamera * ComponentCamera::Clone() const
{
	return new ComponentCamera(*this);
}

void ComponentCamera::InitFrustum()
{
	frustum = new Frustum();
	frustum->type = FrustumType::PerspectiveFrustum;
	frustum->pos = float3::zero;
	frustum->front = -float3::unitZ;
	frustum->up = float3::unitY;
	frustum->nearPlaneDistance = ZNEARDIST* App->renderer->current_scale;
	frustum->farPlaneDistance = ZFARDIST * App->renderer->current_scale;
	frustum->verticalFov = DegToRad(60);
	frustum->horizontalFov = 2.f * atanf(tanf(frustum->verticalFov * 0.5f) * ((float)App->window->width / (float)App->window->height));
	SetFrustumBuffers();
}

void ComponentCamera::SetFrustumBuffers()
{
	// VAO Creation
	if (frustumVAO == 0)
	{
		glGenVertexArrays(1, &frustumVAO);
	}
	glBindVertexArray(frustumVAO);

	float3 corners[8];
	frustum->GetCornerPoints(corners);

	GLfloat vertices[] = {
		corners[0].x, corners[0].y, corners[0].z, 1.0,
		corners[4].x, corners[4].y, corners[4].z, 1.0,
		corners[6].x, corners[6].y, corners[6].z, 1.0,
		corners[2].x, corners[2].y, corners[2].z, 1.0,
		corners[1].x, corners[1].y, corners[1].z, 1.0,
		corners[5].x, corners[5].y, corners[5].z, 1.0,
		corners[7].x, corners[7].y, corners[7].z, 1.0,
		corners[3].x, corners[3].y, corners[3].z, 1.0,
	};
	if (frustumVBO == 0)
	{
		glGenBuffers(1, &frustumVBO);
	}
	glBindBuffer(GL_ARRAY_BUFFER, frustumVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	GLushort elements[] = {
		0, 1, 2, 3,
		4, 5, 6, 7,
		0, 4, 1, 5, 2, 6, 3, 7
	};
	if (frustumEBO == 0)
	{
		glGenBuffers(1, &frustumEBO);
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, frustumEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, frustumVBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,  // attribute
		4,                  // number of elements per vertex, here (x,y,z,w)
		GL_FLOAT,           // the type of each element
		GL_FALSE,           // take our values as-is
		0,                  // no extra data between each position
		0                   // offset of first element
	);

	// Disable VAO
	glBindVertexArray(0);
	glDisableVertexAttribArray(0);

	// Disable VBO and EBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void ComponentCamera::Move(float3 dir)
{
	frustum->Translate(dir*movementSpeed);
}

void ComponentCamera::Rotate(float dx, float dy)
{
	if (dx != 0)
	{
		Quat rotation = Quat::RotateY(math::DegToRad(-dx)).Normalized();
		frustum->front = rotation.Mul(frustum->front).Normalized();
		frustum->up = rotation.Mul(frustum->up).Normalized();
	}
	if (dy != 0)
	{
		Quat rotation = Quat::RotateAxisAngle(frustum->WorldRight(), math::DegToRad(-dy)).Normalized();
		float3 valid_up = rotation.Mul(frustum->up).Normalized();
		if (valid_up.y > 0.0f)
		{
			frustum->up = valid_up;
			frustum->front = rotation.Mul(frustum->front).Normalized();
		}
	}
}


void ComponentCamera::Zoom(float mouseWheel)
{
	if (mouseWheel != 0)
	{
		frustum->Translate(frustum->front * mouseWheel * zoomSpeed);
	}
}

void ComponentCamera::Center()
{
	if (App->scene->selected == nullptr 
		|| App->scene->selected->GetComponent(ComponentType::Renderer) == nullptr) return;

	AABB bbox = App->scene->selected->GetBoundingBox();
	float3 HalfSize = bbox.HalfSize();
	float distX = HalfSize.x / tanf(frustum->horizontalFov*0.5f);
	float distY = HalfSize.y / tanf(frustum->verticalFov*0.5f);
	float camDist = MAX(distX, distY) + HalfSize.z; //camera distance from model

	float3 center = bbox.FaceCenterPoint(5);
	frustum->pos = center + float3(0, 0, camDist);

	frustum->front = -float3::unitZ;
	frustum->up = float3::unitY;
}


void ComponentCamera::Orbit(float dx, float dy)
{
	if (App->scene->selected == nullptr || ImGuizmo::IsUsing()) return;

	AABB bbox = App->scene->selected->GetBoundingBox();
	float3 center = bbox.CenterPoint();

	if (dx != 0)
	{
		Quat rotation = Quat::RotateY(math::DegToRad(-dx)).Normalized();
		frustum->pos = rotation.Mul(frustum->pos - center) + center;
	}
	if (dy != 0)
	{
		Quat rotation = Quat::RotateAxisAngle(frustum->WorldRight(), math::DegToRad(-dy)).Normalized();
		float3 new_pos = rotation.Mul(frustum->pos - center) + center;
		if (!(abs(new_pos.x-center.x) < 0.5f && abs(new_pos.z - center.z) < 0.5f))
		{
			frustum->pos = new_pos;
		}
	}
	LookAt(center);
}

void ComponentCamera::SetAspect(float aspect)
{
	frustum->horizontalFov = 2.f * atanf(tanf(frustum->verticalFov * 0.5f) * aspect);
}

void ComponentCamera::SetFOV(float fov)
{
	float aspect = frustum->AspectRatio();
	frustum->verticalFov = fov;
	SetAspect(aspect);
}

void ComponentCamera::ResetFrustum()
{
	RELEASE(frustum);
	InitFrustum();
}

void ComponentCamera::DrawFrustum(unsigned shader) const
{
	float4x4 model = float4x4::identity.LookAt(-float3::unitZ,frustum->front, frustum->up, float3::unitY);
	model.SetTranslatePart(frustum->pos);
	glUniformMatrix4fv(glGetUniformLocation(shader,
		"model"), 1, GL_TRUE, &model[0][0]);

	float red[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	glUniform4fv(glGetUniformLocation(shader,
		"Vcolor"), 1, red);

	glLineWidth(4.f);
	glBindVertexArray(frustumVAO);
	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, 0);
	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, (GLvoid*)(4 * sizeof(GLushort)));
	glDrawElements(GL_LINES, 8, GL_UNSIGNED_SHORT, (GLvoid*)(8 * sizeof(GLushort)));

	// We disable VAO
	glBindVertexArray(0);
	glLineWidth(1.f);
}

void ComponentCamera::LookAt(float3 target)
{
	float3 dir = (target - frustum->pos).Normalized();
	float3x3 look = float3x3::LookAt(frustum->front, dir, frustum->up, float3::unitY);
	frustum->front = look.Mul(frustum->front).Normalized();
	frustum->up = look.Mul(frustum->up).Normalized();
}


void ComponentCamera::Update() 
{
	if (gameobject == nullptr) return;
	if (gameobject->transform == nullptr) return;

	float4x4 transform = gameobject->GetGlobalTransform();
	frustum->pos = transform.TranslatePart();
	frustum->front = transform.RotatePart().Mul(float3::unitZ).Normalized();
	frustum->up = transform.RotatePart().Mul(float3::unitY).Normalized();
}

void ComponentCamera::DrawProperties()
{
	if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool removed = Component::DrawComponentState();
		if (removed)
		{
			return;
		}

		if (ImGui::Checkbox("Is Main Camera", &isMainCamera))
		{
			if (isMainCamera)
			{
				if (App->scene->maincamera != nullptr)
				{
					App->scene->maincamera->isMainCamera = false;
				}
				App->scene->maincamera = this;
			}
			else
			{
				App->scene->maincamera = nullptr;
			}
		}
		ImGui::DragFloat("Znear", (float*)&frustum->nearPlaneDistance, 0.1f, 0.01f, 1000.f);
		ImGui::DragFloat("Zfar", (float*)&frustum->farPlaneDistance, 0.5f, 1.f, 1000.f);
		float degFov = math::RadToDeg(frustum->verticalFov);
		if (ImGui::SliderFloat("FOV", &degFov, 40, 120))
		{
			frustum->verticalFov = math::DegToRad(degFov);
			frustum->horizontalFov = 2.f * atanf(tanf(frustum->verticalFov*0.5f)*App->window->width / App->window->height);
		}

		ImGui::Separator();
	}
}

void ComponentCamera::Save(JSON_value * value) const
{
	Component::Save(value);
	value->AddFloat("MovementSpeed", movementSpeed);
	value->AddFloat("RotationSpeed", rotationSpeed);
	value->AddFloat("ZoomSpeed", zoomSpeed);
	value->AddFloat("Znear", frustum->nearPlaneDistance);
	value->AddFloat("Zfar", frustum->farPlaneDistance);
	value->AddFloat("vFOV", frustum->verticalFov);
	value->AddFloat("hFOV", frustum->horizontalFov);
	value->AddFloat3("Position", frustum->pos);
	value->AddFloat3("Front", frustum->front);
	value->AddFloat3("Up", frustum->up);
}

void ComponentCamera::Load(const JSON_value & value)
{
	Component::Load(value);
	movementSpeed = value.GetFloat("MovementSpeed");
	rotationSpeed = value.GetFloat("RotationSpeed");
	zoomSpeed = value.GetFloat("ZoomSpeed");
	frustum->nearPlaneDistance = value.GetFloat("Znear");
	frustum->farPlaneDistance = value.GetFloat("Zfar");
	frustum->verticalFov = value.GetFloat("vFOV");
	frustum->horizontalFov = value.GetFloat("hFOV");
	frustum->pos = value.GetFloat3("Position");
	frustum->front = value.GetFloat3("Front");
	frustum->up = value.GetFloat3("Up");
}

float4x4 ComponentCamera::GetViewMatrix() const
{
	float4x4 view = frustum->ViewMatrix();
	return view.Transposed();
}

float4x4 ComponentCamera::GetProjectionMatrix() const
{
	return frustum->ProjectionMatrix().Transposed();
}

LineSegment ComponentCamera::DrawRay(float x, float y) const
{
	return frustum->UnProjectLineSegment(x, y);
}
