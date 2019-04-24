#ifndef _REVERBZONE_
#define _REVERBZONE_

#include "Application.h"
#include "Component.h"
#include "ModuleRender.h"
#include "soloud_echofilter.h"

class GameObject;
class JSON_value;

class ComponentReverbZone :
	public Component
{
public:

	ComponentReverbZone(GameObject* gameobject);
	ComponentReverbZone(const ComponentReverbZone& component);
	~ComponentReverbZone();

	ComponentReverbZone* Clone() const;

	void Save(JSON_value *value) const override;
	void Load(JSON_value *value) override;

	void Update() override;

	void UpdateFilterState();

	void DrawProperties() override;

private:
	void DrawDebugReverb();
	void toolTip(char * tooltip);

public:
	SoLoud::EchoFilter echoFilter;

	float radius = 6 * App->renderer->current_scale;
	float delay = 0.4f;
	float decay = 0.6f;
	float fadeDist = radius - (1.5f * App->renderer->current_scale);

	bool applied = false;
private:
	float decayToUpdate = decay;

};

#endif