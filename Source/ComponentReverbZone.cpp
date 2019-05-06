#include "Application.h"

#include "ComponentReverbZone.h"
#include "ComponentTransform.h"
#include "ComponentAudioListener.h"

#include "ModuleAudioManager.h"
#include "ModuleAudioManager.h"
#include "ModuleTime.h"
#include "ModuleScene.h"

#include "GameObject.h"
#include "JSON.h"

#include "imgui.h"
#include "imgui_internal.h"

#include "debugdraw.h"

float positionGradient;
float dist;

ComponentReverbZone::ComponentReverbZone(GameObject* gameobject) : Component(gameobject, ComponentType::ReverbZone)
{
}

ComponentReverbZone::ComponentReverbZone(const ComponentReverbZone& component) : Component(component)
{
	delay = component.delay;
	decay = component.decay;
	radius = component.radius;
}

ComponentReverbZone::~ComponentReverbZone()
{
	for (int i = 0; i < App->audioManager->reverbZones.size(); ++i) 
	{
		if (App->audioManager->reverbZones[i] == this) {
			App->audioManager->RemoveEchoFilter(i);
			App->audioManager->reverbZones.erase(App->audioManager->reverbZones.begin() + i);
			break;
		}
	}
}

void ComponentReverbZone::UpdateFilterState()
{
	echoFilter.setParams(delay, decayToUpdate);
}

ComponentReverbZone * ComponentReverbZone::Clone() const
{
	return new ComponentReverbZone(*this);
}

void ComponentReverbZone::Update() 
{

	//Update the gradient affectation of the reverb zone 
	if (applied) 
	{
		
		dist = math::Length(App->audioManager->mainListener->gameobject->transform->GetGlobalPosition() -  gameobject->transform->GetGlobalPosition());

		if (dist < fadeDist) positionGradient = 1.f;
		else {
			positionGradient = (dist - fadeDist) / (radius - fadeDist);
			positionGradient = 1.f - positionGradient;
		}	

		decayToUpdate = decay * positionGradient;
		UpdateFilterState();
	}

	fadeDist = math::Min(fadeDist, radius);

	if (App->scene->selected == gameobject) DrawDebugReverb();
}

void ComponentReverbZone::DrawProperties()
{
	if (ImGui::CollapsingHeader("Reverb Zone"))
	{
		bool removed = Component::DrawComponentState();
		if (removed)
		{
			return;
		}

		ImGui::DragFloat("Radius", &radius, 0.1 * App->renderer->current_scale, 0.1f, 200.f * App->renderer->current_scale, "%.1f");
		toolTip("Radius of maximum affectation");
		ImGui::DragFloat("Fade Distance", &fadeDist, 0.1 * App->renderer->current_scale, 0.1f, 200.f * App->renderer->current_scale, "%.1f");
		toolTip("Radius where reverb starts to fade");


		ImGui::NewLine();

		if (App->time->gameState == GameState::STOP) ImGui::DragFloat("Delay", &delay, 0.03f, 0.01f, 5.f, "%.2f");
		else ImGui::Text("Delay: %f. (Not editable while running)", delay, "%.1f");
		toolTip("Time in seconds until the echo\n- Not editable while running");

		ImGui::DragFloat("Decay", &decay, 0.006f, 0.f, 1.f, "%.1f");
		toolTip("Multiplier for the echo");

	}
}

void ComponentReverbZone::Save(JSON_value * value) const
{
	Component::Save(value);
	value->AddFloat("Radius", radius);
	value->AddFloat("Decay", decay);
	value->AddFloat("Delay", delay);
	value->AddFloat("FadeDist", fadeDist);

}

void ComponentReverbZone::Load(JSON_value* value)
{
	Component::Load(value);
	radius = value->GetFloat("Radius");
	delay = value->GetFloat("Delay");
	decay = value->GetFloat("Decay");
	fadeDist = value->GetFloat("FadeDist");
	decayToUpdate = decay;

	UpdateFilterState();
}

void ComponentReverbZone::DrawDebugReverb() 
{
	dd::sphere(gameobject->transform->GetGlobalPosition(), dd::colors::NavajoWhite , radius);
	dd::sphere(gameobject->transform->GetGlobalPosition(), dd::colors::RebeccaPurple, fadeDist);

}

void ComponentReverbZone::toolTip(char * tip) 
{
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text(tip);
		ImGui::EndTooltip();
	}
}