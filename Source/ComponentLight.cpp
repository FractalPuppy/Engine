#include "Application.h"

#include "ModuleProgram.h"
#include "ModuleCamera.h"

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

#define DEBUG_DISTANCE 5

ComponentLight::ComponentLight(GameObject * gameobject) : Component(gameobject, ComponentType::Light)
{
}

ComponentLight::ComponentLight(const ComponentLight & component) : Component(component)
{
	position = component.position;
}

ComponentLight::~ComponentLight()
{
}

void ComponentLight::Update() 
{
	if (gameobject->transform == nullptr) return;
	position = gameobject->transform->position;
	direction = gameobject->transform->rotation*float3::unitZ;
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
		if (ImGui::BeginCombo("",types[(int)type]))
		{
			for (int n = 0; n < LIGHTTYPES; n++)
			{
				bool is_selected = ((int)type == n);
				if (ImGui::Selectable(types[n], is_selected) && (int)type != n)
				{
					type = (LightType)n;
					ResetValues();
				}
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		

		ImGui::ColorEdit3("Color", (float*)&color);

		if (type != LightType::DIRECTIONAL)
		{
			ImGui::Text("Attenuation");
			ImGui::DragFloat("Constant", (float*)&attenuation.x, 0.01f, 0.01f, 10.f);
			ImGui::DragFloat("Linear", (float*)&attenuation.y, 0.01f, 0.01f, 1.f);
			ImGui::DragFloat("Quadratic", (float*)&attenuation.z, 0.01f, 0.0f, 1.f);
		}

		if (type == LightType::SPOT)
		{
			ImGui::Text("Angle");
			ImGui::DragFloat("Inner", (float*)&inner, 0.1f, 0.f, 100.f);
			ImGui::DragFloat("Outer", (float*)&outer, 0.1f, 0.f, 100.f);
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
	
	if (type == LightType::DIRECTIONAL)
	{
		Circle circle(position, direction, 1.f);
		float angle = 0;
		for (unsigned i = 0; i < 8; i++)
		{
			float3 debug_position = circle.GetPoint(angle);

			Line line(debug_position, direction.Normalized());
			float3 farPoint = line.GetPoint(-DEBUG_DISTANCE);

			if (i > 0)
			{
				glVertex3f(line.pos.x, line.pos.y, line.pos.z);
			}
			glVertex3f(line.pos.x, line.pos.y, line.pos.z);
			glVertex3f(farPoint.x, farPoint.y, farPoint.z);
			glVertex3f(line.pos.x, line.pos.y, line.pos.z);

			angle += math::pi*0.25f;
		}

		Line line(circle.GetPoint(0), direction.Normalized());
		glVertex3f(line.pos.x, line.pos.y, line.pos.z); //Close circle
	}
	else if (type == LightType::SPOT)
	{
		float attenuation_distance = GetAttenuationDistance();
		float3 circleCenter = position + attenuation_distance * direction.Normalized();
		float radius = attenuation_distance * tanf(math::DegToRad(inner));
		Circle circle(circleCenter, direction, radius);

		float angle = 0;
		for (unsigned i = 0; i < 8; i++)
		{
			float3 debug_position = circle.GetPoint(angle);

			LineSegment segment(position, debug_position);
			glVertex3f(segment.a.x, segment.a.y, segment.a.z);
			glVertex3f(segment.b.x, segment.b.y, segment.b.z);
			angle += math::pi*0.25f;

		}
	}
	else //POINT
	{
		float attenuation_distance = GetAttenuationDistance();
		Circle circle(position, float3::unitX, attenuation_distance);
		for (unsigned j = 0; j < 3; j++)
		{
			float angle = 0;
			for (unsigned i = 0; i < 8; i++)
			{
				float3 debug_position = circle.GetPoint(angle);

				LineSegment segment(position, debug_position);
				glVertex3f(segment.a.x, segment.a.y, segment.a.z);
				glVertex3f(segment.b.x, segment.b.y, segment.b.z);
				angle += math::pi*0.25f;

			}
			if (j == 0)
			{
				circle.normal = float3::unitY;
			}
			else
			{
				circle.normal = float3::unitZ;
			}
		}
	}
	glEnd();
	glUseProgram(0);
}

void ComponentLight::Load(const JSON_value & value)
{
	Component::Load(value);
	if (gameobject->transform == nullptr) return;

	type = (LightType)value.GetUint("Lighttype");
	position = gameobject->transform->position;
	direction = gameobject->transform->rotation*float3::unitZ;

	if (type != LightType::DIRECTIONAL)
	{
		attenuation = value.GetFloat3("attenuation");
	}

	if (type == LightType::SPOT)
	{
		inner = value.GetFloat("inner");
		outer = value.GetFloat("outer");
	}
}

void ComponentLight::Save(JSON_value * value) const
{
	Component::Save(value);

	value->AddUint("Lighttype", (unsigned)type);

	if (type != LightType::DIRECTIONAL)
	{
		value->AddFloat3("attenuation", attenuation);
	}

	if (type == LightType::SPOT)
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
	float c = attenuation[0] - 10; // 1(constant+linear*distance...) < 0.1
	
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
