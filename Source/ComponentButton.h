#ifndef __ComponentButton_h__
#define __ComponentButton_h__

#include "Component.h"

class ComponentImage;
class ComponentText;
class ComponentTransform2D;

class ComponentButton :	public Component
{
public:
	ComponentButton();
	ComponentButton(GameObject* gameobject);
	ComponentButton(const ComponentButton &copy);
	~ComponentButton();

	void DrawProperties() override;
	void Save(JSON_value* value) const override;
	void Load(JSON_value* value) override;

	void Update() override;

	void AssemblyButton();

	ENGINE_API inline bool IsHovered() { return isHovered; };
	ENGINE_API inline bool IsPressed() { return isPressed; };

	ComponentImage* buttonImage = nullptr;
	ComponentImage* highlightedImage = nullptr;
	ComponentImage* pressedImage = nullptr;
	ComponentText* text = nullptr;
	ComponentTransform2D* rectTransform = nullptr;

	Component* Clone() const override;

	bool isHovered = false;
	bool isPressed = false;
	
};

#endif // __ComponentButton_h__

