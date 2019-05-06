#ifndef _AUDIOLISTENER_
#define _AUDIOLISTENER_

#include "Component.h"

class GameObject;
class JSON_value;

class ComponentAudioListener :
	public Component
{
public:
	ComponentAudioListener(GameObject* gameobject);
	ComponentAudioListener(const ComponentAudioListener& component);
	~ComponentAudioListener();

	ComponentAudioListener* Clone() const;

	void Update() override;

	void Save(JSON_value* value) const override;
	void Load(JSON_value* value) override;


	void DrawProperties() override;

	void SetThisMainListener();

public:
	bool isMainListener = false;


};

#endif // !_AUDIOLISTENER_