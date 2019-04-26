#include "Application.h"

#include "ModuleProgram.h"
#include "ModuleCamera.h"
#include "ModuleRender.h"
#include "ModuleScene.h"
#include "ModuleEditor.h"
#include "ModuleSpacePartitioning.h"

#include "ComponentLight.h"
#include "ComponentTransform.h"
#include "ComponentCamera.h"
#include "Component.h"

#include "GameObject.h"
#include "imgui.h"
#include "JSON.h"
#include "GL/glew.h"
#include "Geometry/Line.h"
#include "Geometry/LineSegment.h"
#include "Geometry/Circle.h"
#include "Math/MathFunc.h"
#include "MathGeoLib/include/Geometry/AABB.h"
#include "debugdraw.h"
#include "AABBTree.h"

#define INITIAL_RADIUS 1000.f

ComponentLight::ComponentLight(GameObject* gameobject) : Component(gameobject, ComponentType::Light)
{
	pointSphere.r = INITIAL_RADIUS;
}

ComponentLight::ComponentLight(const ComponentLight& component) : Component(component)
{
	position = component.position;
	direction = component.direction;
	color = component.color;

	inner = component.inner;
	outer = component.outer;
	range = component.range;
	intensity = component.intensity;
	pointSphere = Sphere(component.pointSphere);
	App->scene->lights.push_back(this);

	produceShadows = component.produceShadows;
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
	ImGui::PushID(this);
	if (ImGui::CollapsingHeader("Light"))
	{
		bool removed = Component::DrawComponentState();
		if (removed)
		{
			ImGui::PopID();
			return;
		}

		ImGui::SameLine();
		Options();

		ImGui::Spacing();
		ImGui::Text("Type");
		const char * types[] = { "Directional","Point", "Spot" };
		if (ImGui::BeginCombo("", types[(int)lightType]))
		{
			int n;
			App->renderer->directionalLight ? n = 1 : n = 0;
			for (; n < LIGHTTYPES; n++)
			{
				bool is_selected = ((int)lightType == n);
				if (ImGui::Selectable(types[n], is_selected) && (int)lightType != n)
				{
					lightType = (LightType)n;
					ResetValues();
					App->spacePartitioning->aabbTreeLighting.ReleaseNode(gameobject->treeNode);
					CalculateGuizmos();
					App->spacePartitioning->aabbTreeLighting.InsertGO(gameobject);
					if (lightType != LightType::DIRECTIONAL && App->renderer->directionalLight == this)
					{
						App->renderer->directionalLight = nullptr;
					}
					else if (lightType == LightType::DIRECTIONAL)
					{
						App->renderer->directionalLight = this;
						produceShadows = false;
					}
				}
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}


		ImGui::ColorEdit3("Color", (float*)&color);

		bool lightDirty = false;

		if (lightType != LightType::DIRECTIONAL)
		{
			ImGui::Text("Attenuation");
			if (lightType == LightType::POINT)
				lightDirty = lightDirty | ImGui::DragFloat("Radius", &pointSphere.r);
			else
				lightDirty = lightDirty | ImGui::DragFloat("Range", &range);
		}
		else
		{
			ImGui::Checkbox("Produce shadows", &produceShadows);
		}

		lightDirty = lightDirty | ImGui::DragFloat("Intensity", &intensity);

		if (lightType == LightType::SPOT)
		{
			ImGui::Text("Angle");
			lightDirty = lightDirty | ImGui::DragFloat("Inner", (float*)&inner, 0.1f, 0.f, 90.f);
			lightDirty = lightDirty | ImGui::DragFloat("Outer", (float*)&outer, 0.1f, 0.f, 90.f);
		}

		if (lightDirty && lightType != LightType::DIRECTIONAL)
		{
			if (App->scene->photoTimer <= 0.f)
			{
				App->scene->TakePhoto();
			}
			App->spacePartitioning->aabbTreeLighting.ReleaseNode(gameobject->treeNode);
			App->spacePartitioning->aabbTreeLighting.InsertGO(gameobject);
		}
		ImGui::Separator();
	}
	ImGui::PopID();
}

void ComponentLight::DrawDebugLight() const
{
	DrawDebug();
}

void ComponentLight::Load(JSON_value* value)
{
	Component::Load(value);
	if (gameobject->transform == nullptr) return;

	lightType = (LightType)value->GetUint("Lighttype");
	color = value->GetColor3("color");
	position = gameobject->transform->GetPosition();
	direction = gameobject->transform->rotation*float3::unitZ;

	if (lightType != LightType::DIRECTIONAL)
	{
		pointSphere.r = value->GetFloat("radius");
		range = value->GetFloat("range");
	}
	else
	{
		App->spacePartitioning->aabbTreeLighting.ReleaseNode(gameobject->treeNode);
	}

	if (lightType == LightType::SPOT)
	{
		inner = value->GetFloat("inner");
		outer = value->GetFloat("outer");
	}

	intensity = value->GetFloat("intensity");
	produceShadows = value->GetInt("produceShadows");
	if (produceShadows)
	{
		App->renderer->directionalLight = this;
	}
}

void ComponentLight::Save(JSON_value* value) const
{
	Component::Save(value);

	value->AddUint("Lighttype", (unsigned)lightType);
	value->AddFloat3("color", color);
	if (lightType != LightType::DIRECTIONAL)
	{
		value->AddFloat("radius", pointSphere.r);
		value->AddFloat("range", range);
	}

	if (lightType == LightType::SPOT)
	{
		value->AddFloat("inner", inner);
		value->AddFloat("outer", outer);
	}

	value->AddFloat("intensity", intensity);
	value->AddInt("produceShadows", produceShadows);
}

void ComponentLight::Paste()
{
	if (App->scene->copyComp != nullptr && App->scene->copyComp->type == type)
	{
		ComponentLight* comp = (ComponentLight*)App->scene->copyComp;

		lightType = comp->lightType;
		color = comp->color;
		intensity = comp->intensity;
		range = comp->range;
		//pointSphere.r = comp->pointSphere.r;
		inner = comp->inner;
		outer = comp->outer;
		CalculateGuizmos();
	}
}

void ComponentLight::Reset()
{
	color = math::float3::one;
	intensity = 1.f;
	range = 200.f;
	pointSphere.r = 200.f;
	inner = 20.f;
	outer = 25.f;
	CalculateGuizmos();
}

ComponentLight* ComponentLight::Clone() const
{
	ComponentLight* newLight = new ComponentLight(gameobject);
	newLight->range = range;
	newLight->inner = inner;
	newLight->outer = outer;
	newLight->intensity = intensity;
	newLight->type = type;
	newLight->lightType = lightType;
	newLight->color = color;
	newLight->direction = direction;
	newLight->pointSphere = Sphere(pointSphere);
	newLight->CalculateGuizmos();
	newLight->produceShadows = produceShadows;
	return newLight;
}

void ComponentLight::ResetValues()
{
	float polar = 0.f;
	float azimuth = 0.f;
	color = float3::one;
	inner = 20.f;
	outer = 25.f;
}

void ComponentLight::DrawDebug() const
{
	switch (lightType)
	{
	case LightType::DIRECTIONAL:
		dd::cone(gameobject->transform->GetGlobalPosition(), direction * App->renderer->current_scale, dd::colors::Green, App->renderer->current_scale, .01f);
		dd::line(gameobject->transform->GetGlobalPosition(), gameobject->transform->GetGlobalPosition() + direction * App->renderer->current_scale * 10, dd::colors::Green);
		break;
	case LightType::POINT:
		dd::sphere(pointSphere.pos, dd::colors::Gold, pointSphere.r);
		dd::aabb(gameobject->bbox.minPoint, gameobject->bbox.maxPoint, dd::colors::BurlyWood);
		break;
	case LightType::SPOT:
		dd::cone(gameobject->transform->GetGlobalPosition(), direction * range, dd::colors::Gold, spotEndRadius, .01f);
		dd::aabb(gameobject->bbox.minPoint, gameobject->bbox.maxPoint, dd::colors::BurlyWood);
	}
}

void ComponentLight::CalculateGuizmos()
{
	if (gameobject != nullptr)
	{
		switch (lightType)
		{
		case LightType::POINT:
			pointSphere.pos = gameobject->transform->GetGlobalPosition();
			gameobject->bbox.SetNegativeInfinity();
			gameobject->bbox.Enclose(pointSphere);
			break;
		case LightType::SPOT:
		{
			spotEndRadius = range * tan(DegToRad(outer));
			pointSphere.pos = gameobject->transform->GetGlobalPosition();
			pointSphere.r = sqrt(pow(spotEndRadius, 2) + pow(range, 2));
			gameobject->bbox.SetNegativeInfinity();
			float3 p0 = pointSphere.pos + gameobject->transform->front * range;
			float3 p1 = p0 + gameobject->transform->right * spotEndRadius;
			float3 p2 = p0 - gameobject->transform->right * spotEndRadius;
			float3 p3 = p0 + gameobject->transform->up * spotEndRadius;
			float3 p4 = p0 - gameobject->transform->up * spotEndRadius;
			float3 points[6] = { pointSphere.pos, p0, p1, p2, p3, p4 };
			gameobject->bbox.SetFrom(points, 6);
			break;
		}
		case LightType::DIRECTIONAL:
			pointSphere.pos = math::float3::zero;
			gameobject->bbox.SetNegativeInfinity();
			gameobject->bbox.Enclose(pointSphere);
			break;
		}
	}
}


