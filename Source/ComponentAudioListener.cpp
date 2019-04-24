#include "ComponentAudioListener.h"
#include "ComponentReverbZone.h"

#include "Application.h"
#include "ModuleAudioManager.h"
#include "ModuleTime.h"


#include "imgui.h"
#include "imgui_internal.h"

#include "ComponentTransform.h"

#include "GameObject.h"
#include "JSON.h"

#include <vector>


ComponentAudioListener::ComponentAudioListener(GameObject* gameobject) : Component(gameobject, ComponentType::AudioListener)
{
}

ComponentAudioListener::ComponentAudioListener(const ComponentAudioListener& component) : Component(component)
{
}

ComponentAudioListener::~ComponentAudioListener()
{
	for (int i = 0; i < App->audioManager->audioListeners.size(); ++i) 
	{
		if (App->audioManager->audioListeners[i] == this) {
			App->audioManager->audioListeners.erase(App->audioManager->audioListeners.begin() + i);
		}
		break;
	}
	if (isMainListener ) 
	{
		App->audioManager->mainListener = nullptr;
		if (App->audioManager->audioListeners.size() > 0) App->audioManager->setMainListener(App->audioManager->audioListeners[0]);
	}
}

void ComponentAudioListener::Update() 
{
	ComponentReverbZone* rz;

	if (App->time->gameState == GameState::RUN) 
	{
		
		for (int i = 0; i < App->audioManager->reverbZones.size(); ++i) 
		{
			rz = App->audioManager->reverbZones[i];
			float dist = math::Length(rz->gameobject->transform->GetGlobalPosition() - gameobject->transform->GetGlobalPosition());

			if (!rz->applied && dist <= rz->radius) App->audioManager->AddEchoFilter(i);
			else if (rz->applied  && dist > rz->radius) App->audioManager->RemoveEchoFilter(i);

		}
	}

}

ComponentAudioListener * ComponentAudioListener::Clone() const
{
	return new ComponentAudioListener(*this);
}

void ComponentAudioListener::DrawProperties() 
{
	if (ImGui::CollapsingHeader("Audio Listener"))
	{
		bool removed = Component::DrawComponentState();
		if (removed)
		{
			return;
		}
		ImGui::NewLine();
		if (ImGui::Checkbox("Main Listener", &isMainListener)) SetThisMainListener();
	}
}

void ComponentAudioListener::SetThisMainListener() 
{
	App->audioManager->setMainListener(this);
}


void ComponentAudioListener::Save(JSON_value* value) const
{
	Component::Save(value);	
	value->AddInt("IsMainListener", isMainListener);
}

void ComponentAudioListener::Load(JSON_value* value)
{
	Component::Load(value);
	isMainListener = value->GetInt("IsMainListener");
	if (isMainListener) App->audioManager->mainListener = this;

}