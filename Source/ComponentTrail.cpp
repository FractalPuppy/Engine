#include "Application.h"

#include "ModuleTime.h"
#include "ModuleParticles.h"
#include "ModuleFileSystem.h"
#include "ModuleTextures.h"
#include "ModuleResourceManager.h"

#include "ComponentTrail.h"
#include "ComponentTransform.h"

#include "GameObject.h"
#include "ResourceTexture.h"

#include "imgui.h"
#include "debugdraw.h"
#include "JSON.h"


#define None "None Selected"

ComponentTrail::ComponentTrail(GameObject* gameobject) : Component(gameobject, ComponentType::Trail)
{
	if (!gameobject->transform)
	{
		gameobject->CreateComponent(ComponentType::Transform);
	}
	modules.push_back(new PMSizeOverTime());
	App->particles->AddTrailRenderer(this);
}

ComponentTrail::ComponentTrail(const ComponentTrail& component) : Component(component)
{	
	if (!gameobject->transform)
	{
		gameobject->CreateComponent(ComponentType::Transform);
	}
	width = component.width;
	duration = component.duration;
	minDistance = component.minDistance;

	textureName = component.textureName;

	App->particles->AddTrailRenderer(this);
}

void ComponentTrail::Update()
{
	unsigned trailPoints = trail.size();
	for (unsigned i = 0u; i < trailPoints; ++i)
	{
		TrailPoint point = trail.front();
		point.remainingTime -= App->time->gameDeltaTime;

		if (point.remainingTime > 0)
		{			
			trail.push(point);
		}

		trail.pop();
	}

	math::float3 pos = gameobject->transform->GetGlobalPosition();
	if (trail.size() == 0 || trail.back().Distance(pos) > minDistance)
	{
		if (trail.size() == 0)
		{
			TrailPoint newPoint(duration, pos, width);
			trail.push(newPoint);
		}
		else
		{			
			TrailPoint newPoint(duration, gameobject->transform->GetGlobalPosition(), trail.back().position, width, gameobject->transform->right);
			trail.push(newPoint);
		}
	}
}

void ComponentTrail::DrawProperties()
{
	ImGui::PushID(this);
	if (ImGui::CollapsingHeader("Trail Renderer", ImGuiTreeNodeFlags_DefaultOpen))
	{
		//texture selector
		if (ImGui::BeginCombo("Texture", textureName.c_str()))
		{
			bool none_selected = (textureName == None);
			if (ImGui::Selectable(None, none_selected))
			{
				if (texture != nullptr)
				{
					unsigned imageUID = App->resManager->FindByExportedFile(textureName.c_str());
					App->resManager->DeleteResource(imageUID);
					texture = nullptr;
				}
				textureName = None;
			}
			if (none_selected)
				ImGui::SetItemDefaultFocus();

			textureFiles = App->resManager->GetResourceNamesList(TYPE::TEXTURE, true);

			for (int n = 0; n < textureFiles.size(); n++)
			{
				bool is_selected = (textureName == textureFiles[n]);
				if (ImGui::Selectable(textureFiles[n].c_str(), is_selected) && !is_selected)
				{
					App->resManager->DeleteResource(App->resManager->FindByExportedFile(textureName.c_str()));
					textureName = textureFiles[n].c_str();
					unsigned imageUID = App->resManager->FindByExportedFile(textureName.c_str());
					texture = (ResourceTexture*)App->resManager->Get(imageUID);
				}
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		if (texture != nullptr)
		{
			ImGui::Image((ImTextureID)texture->gpuID, { 200,200 }, { 0,1 }, { 1,0 });
		}

		ImGui::InputFloat("Width", &width, .01f, .1f);
		ImGui::InputFloat("Duration", &duration, .01f, .1f);
		ImGui::InputFloat("Min. point distance", &minDistance, .01f, .1f);

		for (ParticleModule* pm : modules)
		{
			pm->InspectorDraw();
		}
	}
	ImGui::PopID();
}

void ComponentTrail::Save(JSON_value* value) const
{
	Component::Save(value);
	value->AddFloat("width", width);
	value->AddFloat("duration", duration);
	value->AddFloat("minDistance", minDistance);
	value->AddString("textureName", textureName.c_str());
}

void ComponentTrail::Load(JSON_value* value)
{
	Component::Load(value);
	width = value->GetFloat("width");
	duration = value->GetFloat("duration");
	minDistance = value->GetFloat("minDistance");
	textureName = std::string(value->GetString("textureName"));
	if (textureName != "None Selected")
	{
		texture = (ResourceTexture*)App->resManager->Get(textureName.c_str());
	}
}

ComponentTrail * ComponentTrail::Clone() const
{
	return new ComponentTrail(*this);
}

ComponentTrail::~ComponentTrail()
{
	App->particles->RemoveTrailRenderer(this);
}
