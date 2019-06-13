#ifndef __ComponentButton_h__
#define __ComponentButton_h__

#include "Component.h"

class ComponentImage;
class Text;
class Transform2D;

class Button :	public Component
{
public:
	Button();
	Button(GameObject* gameobject);
	Button(const Button &copy);
	~Button();

	void DrawProperties() override;
	void Save(JSON_value* value) const override;
	void Load(JSON_value* value) override;

	void Update() override;
	void Enable(bool enable) override;

	void AssemblyButton();

	ENGINE_API inline bool IsHovered() { return isHovered; };
	ENGINE_API inline bool IsPressed() { return isKeyDown; };

	ENGINE_API inline bool KeyUp()	 { return isKeyUp; }
	ENGINE_API inline bool KeyDown() { return isKeyDown; }

	ComponentImage* buttonImage = nullptr;
	ComponentImage* highlightedImage = nullptr;
	ComponentImage* pressedImage = nullptr;
	Text* text = nullptr;
	Transform2D* rectTransform = nullptr;

	Component* Clone() const override;

	bool isHovered = false;
	bool isSelected = false;
	bool isKeyDown = false;
	bool isKeyUp = false;
};

#endif // __ComponentButton_h__

