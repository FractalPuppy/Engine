#ifndef _AUDIOMANAGER_
#define _AUDIOMANAGER_

#include "Module.h"
#include "soloud.h"
#include "soloud_wav.h"
#include "Math/float3.h"
#include <vector>
#include <list>

#include "soloud_echofilter.h"

class ComponentAudioListener;
class ComponentReverbZone;
class JSON;
class Soloud;

class ModuleAudioManager :
	public Module
{
public:
	ModuleAudioManager();
	~ModuleAudioManager();

	void DrawGUI() override;

	bool Init(JSON* json) override;
	bool CleanUp() override;

	update_status PostUpdate() override;

	int PlayWAV(SoLoud::AudioSource& wav, bool sound3D);
	void StopWAV(int handler);
	void SetVolume(int handler, float& vol);
	void SetPan(int handler, float& pan);
	void SetLoop(int handler, bool loop);
	void SetPitch(int handler, float& pitch);
	void AddEchoFilter(int i);
	void RemoveEchoFilter(int i);

	void setMainListener(ComponentAudioListener* AL);

private:
	void setMainListener(int i);

public:
	std::vector<ComponentAudioListener*> audioListeners;
	std::vector<ComponentReverbZone*> reverbZones;

	ComponentAudioListener* mainListener = nullptr;

	float masterVolume = 1.f;

private:
	int bushandleRaw;
	int bushandle3D;

	SoLoud::EchoFilter efil;

	SoLoud::Soloud gSoloud; // SoLoud engine
	SoLoud::Bus bus3D;
	SoLoud::Bus busRaw;

	bool busesStoped = false;

};

#endif