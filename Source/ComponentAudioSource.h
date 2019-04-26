#ifndef _AUDIOSOURCE_
#define _AUDIOSOURCE_

#include "Component.h"
#include "soloud.h"
#include "soloud_wav.h"
#include "soloud_wavstream.h"
#include "ModuleRender.h"
#include <string>
#include <vector>


class GameObject;
class JSON_value;
class FileExplorer;

class ComponentAudioSource :
	public Component
{
public:
	ComponentAudioSource(GameObject* gameobject);
	ComponentAudioSource(const ComponentAudioSource& component);
	~ComponentAudioSource();

	void Play();
	void Stop();
	void SetVolume();
	void SetPan();
	void SetLoop();
	void SetPitch();

	ComponentAudioSource* Clone() const;
	void LoadSoundFile(const char* pathFile);

	void Awake();
	void Update() override;
	void DrawProperties() override;

	void Save(JSON_value* value) const override;
	void Load(JSON_value* value) override;

	void DrawDebugSound();

private:
	void UpdateState();
	float Volume3D();
	float Pan3D();

	void toolTip(char * tooltip);


public:
	std::string FXname = "";
	float volume = 1.f;
	bool playOnAwake = true;
	float PAN = 0.f;
	bool loop = false;

	bool streamed = false;

	bool Sound3D = true;
	bool OnlyVolume3D = false;
	float fadeDist = 5.f * App->renderer->current_scale;
	float limit3DPan = 0.5f;
	float rolloff3D = 0.3f;
	float pitch = 1.f;


private:

	SoLoud::Wav wavFX;
	SoLoud::WavStream wavstream;
	int lastHandler = -1;
	std::string path = "";
	
	float volume3d = volume;
	float3 dirListener;

	bool playing = false;	
	bool awaken = false;

	bool mesageSent = false;

	bool filterSeted = false;


};

#endif