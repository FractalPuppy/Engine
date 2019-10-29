#include "ComponentVolumetricLight.h"

#include "Globals.h"
#include "Application.h"
#include "GameObject.h"
#include "ResourceMesh.h"
#include "ResourceMaterial.h"
#include "ModuleScene.h"
#include "ModuleRender.h"
#include "ComponentRenderer.h"
#include "JSON.h"

#include "imgui.h"

ComponentVolumetricLight::ComponentVolumetricLight(GameObject* gameobject) : Component(gameobject, ComponentType::VolumetricLight)
{
	Init();
	renderer = new ComponentRenderer(gameobject);
	renderer->isVolumetricLight = true;		
	renderer->useAlpha = true;
	circle1Radius *= App->renderer->current_scale;
	circle2Radius *= App->renderer->current_scale;
	length *= App->renderer->current_scale;
}

ComponentVolumetricLight::ComponentVolumetricLight(const ComponentVolumetricLight& copy) : Component(copy)
{
	circle1Radius = copy.circle1Radius;
	circle2Radius = copy.circle2Radius;
	length = copy.length;

	Init();
	renderer = (ComponentRenderer*)copy.renderer->Clone();
	renderer->isVolumetricLight = true;
	renderer->useAlpha = true;
	UpdateMesh();
}

ComponentVolumetricLight::~ComponentVolumetricLight()
{
	App->scene->DeleteFromSpacePartition(gameobject);
	RELEASE(mesh);
	RELEASE(renderer);
}

Component* ComponentVolumetricLight::Clone() const
{
	return new ComponentVolumetricLight(*this);
}

void ComponentVolumetricLight::Update()
{
}

void ComponentVolumetricLight::DrawProperties()
{
	ImGui::PushID(this);

	bool active = enabled;

	ImGui::Checkbox("Active", &active); ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.f, 0.6f, 0.6f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.f / 7.0f, 0.7f, 0.7f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.f / 7.0f, 0.8f, 0.8f));

	bool removed = ImGui::Button("Remove");
	ImGui::PopStyleColor(3);

	if (removed) {
		Remove();
		ImGui::PopID();
		return;
	}
	
	if (ImGui::CollapsingHeader("Volumetric Light", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::DragFloat("Base radius", &circle1Radius, 1.f, 0.f, MAX_RADIUS)) 
			UpdateMesh();
		if (ImGui::DragFloat("Origin radius", &circle2Radius, 1.f, 0.f, MAX_RADIUS))
			UpdateMesh();
		if (ImGui::DragFloat("Length", &length, 1.f, 0.f))
			UpdateMesh();
		renderer->DrawProperties();		
	}
	ImGui::PopID();
}

void ComponentVolumetricLight::Save(JSON_value* value) const
{
	Component::Save(value);

	value->AddFloat("circle1Radius", circle1Radius);
	value->AddFloat("circle2Radius", circle2Radius);
	value->AddFloat("length", length);
	JSON_value* hValue = value->CreateValue(rapidjson::kObjectType);
	renderer->Save(hValue);
	value->AddValue("renderer", *hValue);
}

void ComponentVolumetricLight::Load(JSON_value* value)
{
	Component::Load(value);

	renderer->Load(value->GetValue("renderer"));
	circle1Radius = value->GetFloat("circle1Radius");
	circle2Radius = value->GetFloat("circle2Radius");
	length = value->GetFloat("length");

}

void ComponentVolumetricLight::UpdateMesh()
{
	int halfCone = VERT_AMOUNT / 2;
	float cPoints[VERT_AMOUNT];

	unsigned yWatchDog = 1u;

	for (int i = 0; i < halfCone; ++i)
	{
		cPoints[i] = conePoints[i] * circle1Radius;
		if (++yWatchDog % 3u != 0u)
			cPoints[i + halfCone] = conePoints[i + halfCone] * circle2Radius;
		else
			cPoints[i + halfCone] = length;
	}	
	if (mesh)
	{
		RELEASE(mesh);
	}
	mesh = new ResourceMesh(VERT_AMOUNT, &cPoints[0], INDEX_AMOUNT, &coneIndexes[0], UV_AMOUNT, &coneUVs[0]);
	
	renderer->mesh = mesh;
	App->scene->DeleteFromSpacePartition(gameobject);
	gameobject->UpdateBBox();	
	App->scene->AddToSpacePartition(gameobject);
	
	gameobject->movedFlag = true;

}

void ComponentVolumetricLight::Init()
{
	float cPoints[VERT_AMOUNT] = { //2 31 vertices circles of radius 1
		//Circle 1
		1.000f, 0.000f, 0.000f,
		0.981f, 0.000f, -0.195f,
		0.924f, 0.000f, -0.383f,
		0.831f, 0.000f, -0.556f,
		0.707f, 0.000f, -0.707f,
		0.556f, 0.000f, -0.831f,
		0.383f, 0.000f, -0.924f,
		0.195f, 0.000f, -0.981f,
		-0.000f, 0.000f, -1.000f,
		-0.195f, 0.000f, -0.981f,
		-0.383f, 0.000f, -0.924f,
		-0.556f, 0.000f, -0.831f,
		-0.707f, 0.000f, -0.707f,
		-0.831f, 0.000f, -0.556f,
		-0.924f, 0.000f, -0.383f,
		-0.981f, 0.000f, -0.195f,
		-1.000f, 0.000f, 0.000f,
		-0.981f, 0.000f, 0.195f,
		-0.924f, 0.000f, 0.383f,
		-0.831f, 0.000f, 0.556f,
		-0.707f, 0.000f, 0.707f,
		-0.556f, 0.000f, 0.831f,
		-0.383f, 0.000f, 0.924f,
		-0.195f, 0.000f, 0.981f,
		0.000f, 0.000f, 1.000f,
		0.195f, 0.000f, 0.981f,
		0.383f, 0.000f, 0.924f,
		0.556f, 0.000f, 0.831f,
		0.707f, 0.000f, 0.707f,
		0.831f, 0.000f, 0.556f,
		0.924f, 0.000f, 0.383f,
		0.981f, 0.000f, 0.195f,
		1.000f, 0.000f, 0.000f,
		// Circle2
		1.000f, 0.000f, 0.000f,
		0.981f, 0.000f, -0.195f,
		0.924f, 0.000f, -0.383f,
		0.831f, 0.000f, -0.556f,
		0.707f, 0.000f, -0.707f,
		0.556f, 0.000f, -0.831f,
		0.383f, 0.000f, -0.924f,
		0.195f, 0.000f, -0.981f,
		-0.000f, 0.000f, -1.000f,
		-0.195f, 0.000f, -0.981f,
		-0.383f, 0.000f, -0.924f,
		-0.556f, 0.000f, -0.831f,
		-0.707f, 0.000f, -0.707f,
		-0.831f, 0.000f, -0.556f,
		-0.924f, 0.000f, -0.383f,
		-0.981f, 0.000f, -0.195f,
		-1.000f, 0.000f, 0.000f,
		-0.981f, 0.000f, 0.195f,
		-0.924f, 0.000f, 0.383f,
		-0.831f, 0.000f, 0.556f,
		-0.707f, 0.000f, 0.707f,
		-0.556f, 0.000f, 0.831f,
		-0.383f, 0.000f, 0.924f,
		-0.195f, 0.000f, 0.981f,
		0.000f, 0.000f, 1.000f,
		0.195f, 0.000f, 0.981f,
		0.383f, 0.000f, 0.924f,
		0.556f, 0.000f, 0.831f,
		0.707f, 0.000f, 0.707f,
		0.831f, 0.000f, 0.556f,
		0.924f, 0.000f, 0.383f,
		0.981f, 0.000f, 0.195f,
		1.000f, 0.000f, 0.000f,
	};
	memcpy_s(&conePoints[0], sizeof(float) * VERT_AMOUNT, &cPoints[0], sizeof(float) * VERT_AMOUNT);

	int halfUVs = UV_AMOUNT / 2;

	for (unsigned i = 0u; i < halfUVs; i += 2u)
	{
		coneUVs[i] = i / (float)(halfUVs); //Circle1 s coord
		coneUVs[i + 1u] = .95f; //Circle1 t coord
		coneUVs[i + halfUVs] = i / (float)(halfUVs); //Circle2 s coord
		coneUVs[i + halfUVs + 1u] = 0.f; //Circle1 t coord
	}

	//Create triangle strip sequence 
	int cIndexes[INDEX_AMOUNT] = { 31, 0, 32, 1, 33, 2, 34, 3, 35, 4, 36, 5, 37, 6, 38, 7, 39, 8, 40, 9, 41, 10, 42, 11, 43, 12, 44, 13, 45,
						14,	46,	15,	47,	16,	48,	17,	49,	18,	50,	19,	51,	20,	52,	21,	53, 22,	54,	23,	55,	24,	56,	25,	57,	26,	58,	27,	
						59,	28,	60,	29,	61,	30,	62, 31, 63, 32, 65, 65, 65};

	memcpy_s(&coneIndexes[0], sizeof(int) * INDEX_AMOUNT, &cIndexes[0], sizeof(int) * INDEX_AMOUNT);
	
}

