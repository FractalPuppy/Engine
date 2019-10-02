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
class ResourceAudio;

class ComponentAudioSource :
	public Component
{
public:
	ComponentAudioSource(GameObject* gameobject);
	ComponentAudioSource(const ComponentAudioSource& component);
	~ComponentAudioSource();

	ENGINE_API void Play();
	ENGINE_API void Stop();
	ENGINE_API void SetVolume(float newVol);
	ENGINE_API void SetPan(float newPan);
	ENGINE_API void SetLoop(bool newLoop);
	ENGINE_API void SetPitch(float newPitch);

	ComponentAudioSource* Clone() const;

	void Awake();
	void Update() override;
	void DrawProperties() override;

	void Save(JSON_value* value) const override;
	void Load(JSON_value* value) override;

	void DrawDebugSound();

	ENGINE_API void UpdateState();

private:

	void UpdateAudiosList();
	
	float Volume3D();
	float Pan3D();

	void toolTip(char* tooltip);


public:

	ResourceAudio* audio = nullptr;

	float volume = 1.f;
	bool playOnAwake = true;
	float PAN = 0.f;
	bool loop = false;

	bool Sound3D = true;
	bool OnlyVolume3D = false;
	float fadeDist = 5.f * App->renderer->current_scale;
	float limit3DPan = 0.5f;
	float rolloff3D = 0.3f;
	float pitch = 1.f;


private:
	
	float volume3d = volume;
	float3 dirListener;
	int lastHandler = -1;

	bool playing = false;	
	bool awaken = false;

	bool mesageSent = false;

	bool filterSeted = false;

	std::vector<std::string> audioFiles;
};

#endif