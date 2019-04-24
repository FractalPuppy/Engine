#include "Application.h"

#include "ComponentAudioSource.h"
#include "ComponentTransform.h"
#include "ComponentAudioListener.h"
#include "ComponentReverbZone.h"


#include "FileExplorer.h"

#include "GameObject.h"
#include "JSON.h"

#include "ModuleFileSystem.h"
#include "ModuleAudioManager.h"
#include "ModuleTime.h"
#include "ModuleScene.h"


#include "imgui.h"
#include "imgui_internal.h"

#include "debugdraw.h"

ComponentAudioSource::ComponentAudioSource(GameObject* gameobject) : Component(gameobject, ComponentType::AudioSource)
{
}

ComponentAudioSource::ComponentAudioSource(const ComponentAudioSource& component) : Component(component)
{
	path = component.path;
	FXname = component.FXname;
	playOnAwake = component.playOnAwake;
	volume = component.volume;
	PAN = component.PAN;
	loop = component.loop;
	streamed = component.streamed;
	Sound3D = component.Sound3D;
	fadeDist = component.fadeDist;
	limit3DPan = component.limit3DPan;
	rolloff3D = component.rolloff3D;
	pitch = component.pitch;

}

ComponentAudioSource::~ComponentAudioSource()
{
}

ComponentAudioSource * ComponentAudioSource::Clone() const
{
	return new ComponentAudioSource(*this);
}

void ComponentAudioSource::Play() 
{
	if (enabled) 
	{
		Stop();
		if (!streamed) lastHandler = App->audioManager->PlayWAV(wavFX, Sound3D);
		else lastHandler = App->audioManager->PlayWAV(wavstream, Sound3D);
	}
}

void ComponentAudioSource::Stop() 
{
	App->audioManager->StopWAV(lastHandler);	
}

void ComponentAudioSource::SetVolume() 
{
	if (Sound3D) App->audioManager->SetVolume(lastHandler, volume3d);
	else App->audioManager->SetVolume(lastHandler, volume);
}

void ComponentAudioSource::SetPan() 
{
	App->audioManager->SetPan(lastHandler, PAN);
}

void ComponentAudioSource::SetLoop() 
{
	App->audioManager->SetLoop(lastHandler, loop);
}

void ComponentAudioSource::SetPitch()
{
	App->audioManager->SetPitch(lastHandler, pitch);
}


void ComponentAudioSource::UpdateState() 
{
	SetVolume();
	SetPan();
	SetLoop();
	SetPitch();
}


void ComponentAudioSource::LoadSoundFile(const char* pathAudio)
{
	
	std::vector<std::string> extensions = { ".wav", ".ogg", ".mp3" };
	int ret;
	if (!streamed) ret = wavFX.load(pathAudio);
	else ret = wavstream.load(pathAudio);

	for(int i = 0; i < extensions.size() && (ret != 0); ++i) 
	{
		std::string pathAux = pathAudio;
		std::string folder = "Assets/";
		std::string unionPath = (folder + pathAux + extensions[i]);
		const char * loadPath = unionPath.c_str();
		if (!streamed) ret = wavFX.load(loadPath);
		else ret = wavstream.load(loadPath);
	}

	//SO_NO_ERROR = 0, // No error
	//INVALID_PARAMETER = 1, // Some parameter is invalid
	//FILE_NOT_FOUND = 2,    // File not found
	//FILE_LOAD_FAILED = 3,  // File found, but could not be loaded

	if (ret != 0) 
	{
		LOG("ERROR: Audio Manager code: %i", ret);		
	}
	else 
	{
		LOG("Audio Manager: FX %s loaded", pathAudio);
		path = pathAudio;

		ModuleFileSystem* fileSys = new ModuleFileSystem();
		FXname = fileSys->GetFilename(path);
	}
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
		if (App->audioManager->audioListeners.size() > 0) 
		{
			// Awake function
			if (!awaken) Awake();

			if (Sound3D) 
			{
				volume3d = Volume3D();
				PAN = Pan3D();				
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
		//ImGui::Text("File: \"%s\"", FXname.c_str());

		//Blocked till file exlorer returns the selected path
		
		/*if (ImGui::Button("Load Audio"))
		{
			fileExplorer->currentOperation = MenuOperations::GETPATH;
			fileExplorer->extensionToFilter = FILETYPE::AUDIO;
			std::string audiosPath = AUDIOS;
			audiosPath = audiosPath.substr(0, audiosPath.size() - 1);
			fileExplorer->SetPath(*audiosPath.c_str());
			sprintf_s(fileExplorer->title, "Load Audio");
			fileExplorer->openFileExplorer = true;
		}*/

		if (ImGui::BeginCombo("", path.c_str())) 
		{

			std::set<std::string> files;

			App->fsystem->ListFiles(AUDIOS, files);


			for each (std::string file in files)
			{
				bool is_selected = (path.c_str() == file);
				if (ImGui::Selectable(file.c_str(), is_selected) && path.c_str() != file)
				{
					const char * asd = file.c_str();
					LoadSoundFile((AUDIOS + file).c_str());
				}
				if (is_selected)
				{
					ImGui::SetItemDefaultFocus();
				}

			}
		
			ImGui::EndCombo();
		}


		ImGui::NewLine();
		if (ImGui::Checkbox("Streamed", &streamed)) 
		{
			LoadSoundFile(this->path.c_str());
		}
		toolTip("Streamed audio files will load into memory gradualy\n- Uncheck if its an FX");


		ImGui::SliderFloat("Volume", &volume, 0, 2, "%.1f");
		if (!Sound3D) {
			ImGui::SliderFloat("Stereo Pan", &PAN, -1.f, 1.f, "%.2f");
			toolTip("Manual panning (Only for the non 3D sounds");
		}
	
		ImGui::SliderFloat("Pitch", &pitch, 0.01f, 2.f, "%.2f");
		toolTip("Pitch is based on the playing speed");
		
		ImGui::Checkbox("Play On Awake", &playOnAwake);
		toolTip("The audio will play when the game starts");

		if (ImGui::Checkbox("Loop", &loop)) SetLoop();
		toolTip("Set whether the audio clip replays after it finishes");

		ImGui::Checkbox("3D Sound", &Sound3D);
		toolTip("Enables the 3D sound options");

		if (Sound3D) 
		{
			ImGui::Text("3D Audio Settings:");
			ImGui::DragFloat("FadeDistance", &fadeDist, 0.1 * App->renderer->current_scale, 0.1f, 200.f * App->renderer->current_scale, "%.1f");
			toolTip("Distance where the sound starts fading");

			ImGui::DragFloat("Rolloff", &rolloff3D, 0.0005 * App->renderer->current_scale, 0.f, 0.2f * App->renderer->current_scale, "%.1f");
			toolTip("Modifier of the attenuation curve");

			ImGui::SliderFloat("Limit PAN", &limit3DPan, 0.f, 1.f, "%.2f");
			toolTip("Limits the maximum pan balancing on both ears");			
		}
	}	
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
	value->AddFloat("Volume", volume);
	value->AddInt("PlayOnAwake", playOnAwake);
	value->AddString("Path", path.c_str());
	value->AddFloat("LimitPan", limit3DPan);
	value->AddInt("Loop", loop);
	value->AddFloat("Rolloff", rolloff3D);
	value->AddInt("Streamed", streamed);
	value->AddFloat("Pitch", pitch);
}

void ComponentAudioSource::Load(JSON_value* value)
{
	Component::Load(value);
	volume = value->GetFloat("Volume");
	playOnAwake = value->GetInt("PlayOnAwake");
	path = value->GetString("Path");
	if (path == "") path = "No Audio Selected";
	limit3DPan = value->GetFloat("LimitPan");
	loop = value->GetInt("Loop");
	rolloff3D = value->GetFloat("Rolloff");
	streamed = value->GetInt("Streamed");
	pitch = value->GetFloat("Pitch");

	if (path != "No Audio Selected") LoadSoundFile(path.c_str());

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
