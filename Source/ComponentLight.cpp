#include "Application.h"

#include "ModuleProgram.h"
#include "ModuleCamera.h"
#include "ModuleRender.h"
#include "ModuleScene.h"

#include "ComponentLight.h"
#include "ComponentTransform.h"

#include "GameObject.h"
#include "Imgui/imgui.h"
#include "JSON.h"
#include "GL/glew.h"
#include "Geometry/Line.h"
#include "Geometry/LineSegment.h"
#include "Geometry/Circle.h"
#include "Math/MathFunc.h"
#include "debugdraw.h"

#define DEBUG_DISTANCE 5.0f

ComponentLight::ComponentLight(GameObject * gameobject) : Component(gameobject, ComponentType::Light)
{
}

ComponentLight::ComponentLight(const ComponentLight & component) : Component(component)
{
	position = component.position;
	direction = component.direction;
	color = component.color;

	attenuation = component.attenuation;
	inner = component.inner;
	outer = component.outer;
	App->scene->lights.push_back(this);
}

ComponentLight::~ComponentLight()
{
	App->scene->lights.remove(this);
}

void ComponentLight::Update() 
{
	if (gameobject->transform == nullptr) return;
	position = gameobject->transform->global.TranslatePart(); 

	direction = -(gameobject->transform->rotation*float3::unitZ).Normalized();
}

void ComponentLight::DrawProperties()
{
	if (ImGui::CollapsingHeader("Light"))
	{
		bool removed = Component::DrawComponentState();
		if (removed)
		{
			return;
		}
		ImGui::Separator();
		ImGui::Text("Type");
		const char * types[] = {"Directional","Point", "Spot"};
		if (ImGui::BeginCombo("",types[(int)lightType]))
		{
			for (int n = 0; n < LIGHTTYPES; n++)
			{
				bool is_selected = ((int)lightType == n);
				if (ImGui::Selectable(types[n], is_selected) && (int)lightType != n)
				{
					lightType = (LightType)n;
					ResetValues();
				}
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		

		ImGui::ColorEdit3("Color", (float*)&color);

		if (lightType != LightType::DIRECTIONAL)
		{
			ImGui::Text("Attenuation");
			ImGui::DragFloat("Constant", (float*)&attenuation.x, 0.01f, 0.001f, 10.f,"%.12f");
			ImGui::DragFloat("Linear", (float*)&attenuation.y, 0.0001f, 0.0001f, 1.f, "%.12f");
			ImGui::DragFloat("Quadratic", (float*)&attenuation.z, 0.0001f, 0.0f, 1.f, "%.12f");
		}

		if (lightType == LightType::SPOT)
		{
			ImGui::Text("Angle");
			ImGui::DragFloat("Inner", (float*)&inner, 0.1f, 0.f, 90.f);
			ImGui::DragFloat("Outer", (float*)&outer, 0.1f, 0.f, 90.f);
		}
	}
}

void ComponentLight::DrawDebugLight() const
{
	unsigned shader = App->program->defaultShader->id;
	glUseProgram(shader);
	math::float4x4 model = math::float4x4::identity;
	glUniformMatrix4fv(glGetUniformLocation(shader,
		"model"), 1, GL_TRUE, &model[0][0]);
	glLineWidth(3.0f);

	float red[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
	glUniform4fv(glGetUniformLocation(shader,
		"Vcolor"), 1, red);

	glBegin(GL_LINES);
	
	if (lightType == LightType::DIRECTIONAL)
	{
		DrawDebugDirectional();
	}
	else if (lightType == LightType::SPOT)
	{
		DrawDebugSpot();
	}
	else //POINT
	{
		DrawDebugPoint();
	}
	glEnd();
	glUseProgram(0);
}

void ComponentLight::Load(const JSON_value & value)
{
	Component::Load(value);
	if (gameobject->transform == nullptr) return;

	lightType = (LightType)value.GetUint("Lighttype");
	color = value.GetColor3("color");
	position = gameobject->transform->position;
	direction = gameobject->transform->rotation*float3::unitZ;

	if (lightType != LightType::DIRECTIONAL)
	{
		attenuation = value.GetFloat3("attenuation");
	}

	if (lightType == LightType::SPOT)
	{
		inner = value.GetFloat("inner");
		outer = value.GetFloat("outer");
	}
}

void ComponentLight::Save(JSON_value * value) const
{
	Component::Save(value);

	value->AddUint("Lighttype", (unsigned)lightType);
	value->AddFloat3("color", color);

	if (lightType != LightType::DIRECTIONAL)
	{
		value->AddFloat3("attenuation", attenuation);
	}

	if (lightType == LightType::SPOT)
	{
		value->AddFloat("inner", inner);
		value->AddFloat("outer", outer);
	}
}

ComponentLight * ComponentLight::Clone() const
{
	return new ComponentLight(*this);
}

void ComponentLight::ResetValues()
{
	float polar = 0.f; 
	float azimuth = 0.f; 
	color = float3::one;
	attenuation = float3(0.1f, 0.1f, 0.1f);
	inner = 20.f;
	outer = 25.f;
}

float ComponentLight::GetAttenuationDistance() const
{
	float a = attenuation[2];
	float b = attenuation[1];
	float c = attenuation[0] - 2.5f; // 1/(constant+linear*distance...) < 0.4
	
	float delta = b * b - 4 * a * c;
	if (delta < 0)
	{
		LOG("Error in Attenuation Distance");
		return 0.f;
	}

	if (a == 0) //quadratic is 0
	{
		return -c / b;
	}
	return (-b + sqrt(delta)) / (2*a);

}

void ComponentLight::DrawDebugDirectional() const
{
	math::Circle circle(position, direction, App->renderer->current_scale);
	float angle = 0;
	for (unsigned i = 0; i < 8; i++)
	{
		math::float3 debug_position = circle.GetPoint(angle);

		math::Line line(debug_position, direction.Normalized());
		math::float3 farPoint = line.GetPoint(-DEBUG_DISTANCE * App->renderer->current_scale);

		//Getting next point from the angle
		dd::line(line.pos, farPoint, dd::colors::Yellow);
		angle += math::pi * 0.25f;

		// Drawing the circle of the gizmo
		math::float3 next_debug_position = circle.GetPoint(angle);
		math::Line next_circle_line(next_debug_position, direction.Normalized());
		dd::line(line.pos, next_circle_line.pos , dd::colors::Yellow);
	}
}

void ComponentLight::DrawDebugSpot() const
{
	float attenuation_distance = GetAttenuationDistance();
	math::float3 circleCenter = position + attenuation_distance * direction.Normalized();
	float radius = attenuation_distance * tanf(math::DegToRad(outer));
	math::Circle circle(circleCenter, direction, radius);
	DrawDebugArea(circle);
}

void ComponentLight::DrawDebugPoint() const
{
	float attenuation_distance = GetAttenuationDistance();
	math::Circle circle(position, math::float3::unitX, attenuation_distance);
	for (unsigned j = 0; j < 3; j++)
	{
		DrawDebugArea(circle);
		if (j == 0)
		{
			circle.normal = math::float3::unitY;
		}
		else
		{
			circle.normal = math::float3::unitZ;
		}
	}
}

void ComponentLight::DrawDebugArea(const math::Circle &circle) const
{
	float angle = 0.f;
	for (unsigned i = 0; i < 8; i++)
	{
		math::float3 debug_position = circle.GetPoint(angle);
		math::LineSegment segment(position, debug_position);
		dd::line(segment.a, segment.b, dd::colors::Yellow);
		angle += math::pi * 0.25f;
	}
}
