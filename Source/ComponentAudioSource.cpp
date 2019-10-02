#include "Application.h"

#include "ComponentAudioSource.h"
#include "ComponentTransform.h"
#include "ComponentAudioListener.h"
#include "ComponentReverbZone.h"

#include "GameObject.h"

#include "ModuleFileSystem.h"
#include "ModuleAudioManager.h"
#include "ModuleTime.h"
#include "ModuleScene.h"
#include "ModuleResourceManager.h"

#include "ResourceAudio.h"

#include "FileExplorer.h"

#include "JSON.h"
#include "HashString.h"
#include "imgui.h"
#include "imgui_internal.h"

#include "debugdraw.h"

#define None "None Selected"

ComponentAudioSource::ComponentAudioSource(GameObject* gameobject) : Component(gameobject, ComponentType::AudioSource)
{
	if (audioFiles.empty())
	{
		UpdateAudiosList();
	}
}

ComponentAudioSource::ComponentAudioSource(const ComponentAudioSource& component) : Component(component)
{
	if (component.audio != nullptr)
	{
		audio = (ResourceAudio*)App->resManager->Get(component.audio->GetUID());
	}

	playOnAwake = component.playOnAwake;
	volume = component.volume;
	PAN = component.PAN;
	loop = component.loop;
	Sound3D = component.Sound3D;
	fadeDist = component.fadeDist;
	limit3DPan = component.limit3DPan;
	rolloff3D = component.rolloff3D;
	pitch = component.pitch;

	if (audioFiles.empty())
	{
		UpdateAudiosList();
	}

}

ComponentAudioSource::~ComponentAudioSource()
{
	Stop();
	if (audio != nullptr)
	{
		App->resManager->DeleteResource(audio->GetUID());
		audio = nullptr;
	}
}

ComponentAudioSource* ComponentAudioSource::Clone() const
{
	return new ComponentAudioSource(*this);
}

void ComponentAudioSource::Play() 
{
	if (enabled && audio != nullptr) 
	{
		Stop();
		if (audio != nullptr)
		{
			if (!audio->streamed) 
				lastHandler = App->audioManager->PlayWAV(audio->wavFX, Sound3D && !OnlyVolume3D);
			else 
				lastHandler = App->audioManager->PlayWAV(audio->wavstream, Sound3D && !OnlyVolume3D);
		}
		else
		{
			LOG("Error: missing audio file in ComponentAudioSource of the GO named: %s", gameobject->name.c_str());
		}

	}
}

void ComponentAudioSource::Stop() 
{
	App->audioManager->StopWAV(lastHandler);	
}

void ComponentAudioSource::SetVolume(float newVol) 
{
	volume = newVol;
}

void ComponentAudioSource::SetPan(float newPan) 
{
	PAN = newPan;
}

void ComponentAudioSource::SetLoop(bool newLoop) 
{
	loop = newLoop;
}

void ComponentAudioSource::SetPitch(float newPitch)
{
	pitch = newPitch;
}


void ComponentAudioSource::UpdateState() 
{
	if (Sound3D) App->audioManager->SetVolume(lastHandler, volume3d);
	else App->audioManager->SetVolume(lastHandler, volume);

	App->audioManager->SetPan(lastHandler, PAN);
	
	App->audioManager->SetLoop(lastHandler, loop);
	App->audioManager->SetPitch(lastHandler, pitch);
}

void ComponentAudioSource::Awake() 
{
	Stop();

	if (playOnAwake) Play();

	awaken = true;
}

void ComponentAudioSource::Update() 
{
	if (App->time->gameState == GameState::RUN) 
	{
		if (!App->audioManager->audioListeners.empty()) 
		{
			// Awake function
			if (!awaken) Awake();

			if (Sound3D) 
			{
				volume3d = Volume3D();
				if (!OnlyVolume3D) PAN = Pan3D();				
			}
		}
		else if (!mesageSent) 
		{
			LOG("ERROR: No audio listener on the scene");
			Stop();
			mesageSent = true;
		}
	}

	else if (App->time->gameState == GameState::STOP) 
	{
		if (awaken) awaken = false;

		mesageSent = false;
	}
	if (App->scene->selected == this->gameobject) DrawDebugSound();

	UpdateState();
	
}

void ComponentAudioSource::DrawProperties()
{

	if (ImGui::CollapsingHeader("Audio Source"))
	{
		bool removed = Component::DrawComponentState();
		if (removed)
		{
			return;
		}

		// Audio selector
		if (ImGui::BeginCombo("Audio", audio != nullptr ? audio->GetName() : None))
		{
			bool none_selected = (audio == nullptr);
			if (ImGui::Selectable(None, none_selected))
			{
				if (audio != nullptr)
				{
					App->resManager->DeleteResource(audio->GetUID());
					audio = nullptr;
				}
			}
			if (none_selected)
				ImGui::SetItemDefaultFocus();
			for (int n = 0; n < audioFiles.size(); n++)
			{
				bool is_selected = (audio != nullptr && (HashString(audio->GetName()) == HashString(audioFiles[n].c_str())));
				if (ImGui::Selectable(audioFiles[n].c_str(), is_selected) && !is_selected)
				{
					// Delete previous audio
					if (audio != nullptr)
						App->resManager->DeleteResource(audio->GetUID());

					audio = (ResourceAudio*)App->resManager->GetByName(audioFiles[n].c_str(), TYPE::AUDIO);
				}
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		
		}
		if (ImGui::Button("Refresh List"))
		{
			UpdateAudiosList();
		}

		ImGui::NewLine();

		ImGui::SliderFloat("Volume", &volume, 0, 2, "%.1f");
		if (!Sound3D) {
			ImGui::SliderFloat("Stereo Pan", &PAN, -1.f, 1.f, "%.2f");
			toolTip("Manual panning (Only for the non 3D sounds");
		}
	
		ImGui::SliderFloat("Pitch", &pitch, 0.01f, 2.f, "%.2f");
		toolTip("Pitch is based on the playing speed");
		
		ImGui::Checkbox("Play On Awake", &playOnAwake);
		toolTip("The audio will play when the game starts");

		if (ImGui::Checkbox("Loop", &loop)) UpdateState();
		toolTip("Set whether the audio clip replays after it finishes");

		ImGui::Checkbox("3D Sound", &Sound3D);
		toolTip("Enables the 3D sound options");

		if (Sound3D) 
		{
			ImGui::Text("3D Audio Settings:");

			if(ImGui::Checkbox("Only 3D Volume", &OnlyVolume3D)) PAN = 0.f;
			toolTip("If checked, 3D only affects the volume");

			ImGui::DragFloat("FadeDistance", &fadeDist, 0.1 * App->renderer->current_scale, 0.1f, 200.f * App->renderer->current_scale, "%.1f");
			toolTip("Distance where the sound starts fading");

			ImGui::DragFloat("Rolloff", &rolloff3D, 0.0005 * App->renderer->current_scale, 0.f, 0.2f * App->renderer->current_scale, "%.1f");
			toolTip("Modifier of the attenuation curve");

			ImGui::SliderFloat("Limit PAN", &limit3DPan, 0.f, 1.f, "%.2f");
			toolTip("Limits the maximum pan balancing on both ears");			
		}
	}	
}

void ComponentAudioSource::UpdateAudiosList()
{
	audioFiles.clear();
	audioFiles = App->resManager->GetResourceNamesList(TYPE::AUDIO, true);
}

void ComponentAudioSource::DrawDebugSound() 
{
	assert(gameobject->transform != nullptr);
	if (Sound3D) 
	{
		dd::sphere(gameobject->transform->GetGlobalPosition(), dd::colors::Blue, fadeDist + 100 * App->renderer->current_scale * fadeDist / (fadeDist + rolloff3D * 12 * App->renderer->current_scale));
		dd::sphere(gameobject->transform->GetGlobalPosition(), dd::colors::BlueViolet, fadeDist);
	}
}

void ComponentAudioSource::Save(JSON_value* value) const
{
	Component::Save(value);
	value->AddUint("audioUID", (audio != nullptr) ? audio->GetUID() : 0u);
	value->AddFloat("Volume", volume);
	value->AddInt("PlayOnAwake", playOnAwake);
	value->AddInt("Sound3D", Sound3D);
	value->AddFloat("LimitPan", limit3DPan);
	value->AddFloat("FadeDist", fadeDist);
	value->AddInt("Loop", loop);
	value->AddFloat("Rolloff", rolloff3D);
	value->AddFloat("Pitch", pitch);
}

void ComponentAudioSource::Load(JSON_value* value)
{
	Component::Load(value);
	unsigned uid = value->GetUint("audioUID");
	audio = (ResourceAudio*)App->resManager->Get(uid);
	volume = value->GetFloat("Volume");
	playOnAwake = value->GetInt("PlayOnAwake");
	Sound3D = value->GetInt("Sound3D");
	limit3DPan = value->GetFloat("LimitPan");
	fadeDist = value->GetFloat("FadeDist");
	loop = value->GetInt("Loop");
	rolloff3D = value->GetFloat("Rolloff");
	pitch = value->GetFloat("Pitch");
}

float ComponentAudioSource::Volume3D()
{	
	assert(gameobject->transform != nullptr);

	float Pdist = math::Length(App->audioManager->mainListener->gameobject->transform->GetGlobalPosition() - gameobject->transform->GetGlobalPosition());

	if (Pdist < fadeDist) return volume;
	float Pnorm = math::Abs(Pdist - fadeDist);


	float ret = fadeDist / (fadeDist + rolloff3D * Pnorm);
	return  volume * ret;
}

float ComponentAudioSource::Pan3D()
{
	float3 leftListener = App->audioManager->mainListener->gameobject->transform->rotation * float3(-1.f, 0.f ,0.f);
	float3 sourceListener = gameobject->transform->GetGlobalPosition() - App->audioManager->mainListener->gameobject->transform->GetGlobalPosition();

	float dotP = math::Dot(leftListener.Normalized(), sourceListener.Normalized());

	dotP = math::Min(1.f - limit3DPan, dotP);
	dotP = math::Max(-1.f + limit3DPan, dotP);

	return dotP;
}

void ComponentAudioSource::toolTip(char * tip) {
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text(tip);
		ImGui::EndTooltip();
	}
}
