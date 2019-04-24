#include "Application.h"

#include "ModuleAudioManager.h"
#include "ModuleTime.h"

#include "ComponentAudioListener.h"
#include "ComponentReverbZone.h"

#include "GameObject.h"

#include "imgui.h"


ModuleAudioManager::ModuleAudioManager()
{
}


ModuleAudioManager::~ModuleAudioManager()
{
}

update_status ModuleAudioManager::PostUpdate() 
{

	if (App->time->gameState == GameState::RUN) busesStoped = false;

	else if (App->time->gameState == GameState::STOP && !busesStoped)
	{ 
		for (int i = 0; i < reverbZones.size(); ++i)  RemoveEchoFilter(i);
		busesStoped = true;
	}

	gSoloud.update3dAudio();
	return UPDATE_CONTINUE;
}

bool ModuleAudioManager::Init(JSON* json) 
{
	gSoloud.init();
	bushandleRaw = gSoloud.play(busRaw);
	bushandle3D = gSoloud.play(bus3D);

	gSoloud.setGlobalVolume(masterVolume);

	return true;
}

int ModuleAudioManager::PlayWAV(SoLoud::AudioSource& wav, bool sound3D) 
{

	if (sound3D) return bus3D.play(wav);
	else return busRaw.play(wav);
}

void ModuleAudioManager::AddEchoFilter(int i)
{
	bus3D.setFilter(i, &reverbZones[i]->echoFilter);
	reverbZones[i]->applied = true;
}

void ModuleAudioManager::RemoveEchoFilter(int i)
{
	bus3D.setFilter(i, NULL);
	reverbZones[i]->applied = false;

}

void ModuleAudioManager::StopWAV(int handler) 
{
	return gSoloud.stop(handler);
}

void ModuleAudioManager::SetVolume(int handler, float& vol) 
{
	gSoloud.setVolume(handler, vol);
}

void ModuleAudioManager::SetPan(int handler, float& pan) 
{
	gSoloud.setPan(handler, pan);
}

void ModuleAudioManager::SetLoop(int handler, bool loop)
{
	gSoloud.setLooping(handler, loop);
	
}

void ModuleAudioManager::SetPitch(int handler, float& pitch)
{
		gSoloud.setRelativePlaySpeed(handler, pitch);
}

void ModuleAudioManager::DrawGUI() 
{
	if (ImGui::SliderFloat("Master Volume", &masterVolume, 0.f, 2.f, "%.2f")) 	gSoloud.setGlobalVolume(masterVolume);

	ImGui::NewLine();
	if (mainListener != nullptr) ImGui::Text("Main Listener: %s", mainListener->gameobject->name.c_str());
	else ImGui::Text("Main Listener: No Audio Listener on the secene");
}

void ModuleAudioManager::setMainListener(int newMainListener) 
{
	for (int i = 0; i < audioListeners.size(); ++i) audioListeners[i]->isMainListener = false;

	audioListeners[newMainListener]->isMainListener = true;
	mainListener = audioListeners[newMainListener];
}

void ModuleAudioManager::setMainListener(ComponentAudioListener* AL) 
{
	for (int i = 0; i < audioListeners.size(); ++i) if (AL == audioListeners[i]) setMainListener(i);
}

bool ModuleAudioManager::CleanUp() 
{
	gSoloud.deinit();
	return true;
}