#ifndef __ComponentButton_h__
#define __ComponentButton_h__

#include "Component.h"

class ComponentImage;
class Text;
class Transform2D;
class ResourceTexture;

enum class ButtonState
{
	NONE = 0,
	DOWN,
	REPEAT,
	UP
};

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

	void PreUpdate() override;
	void Update() override;
	void Enable(bool enable) override;

	void AssemblyButton();

	ENGINE_API bool IsHovered();
	ENGINE_API inline bool IsPressed() { return state == ButtonState::DOWN; };

	ENGINE_API inline bool KeyUp()	 { return state == ButtonState::UP; }
	ENGINE_API inline bool KeyDown() { return state == ButtonState::DOWN; }

	ENGINE_API void UpdateImageByName(std::string name);
	ENGINE_API void UpdateImageByResource(ResourceTexture* name);

	ComponentImage* buttonImage = nullptr;
	ComponentImage* highlightedImage = nullptr;
	ComponentImage* pressedImage = nullptr;
	Text* text = nullptr;
	Transform2D* rectTransform = nullptr;

	Component* Clone() const override;

	bool isHovered = false;
	bool isSelected = false;

	ButtonState state = ButtonState::NONE;

	bool hoverDetectionMouse1 = true;
	bool hoverDetectionMouse3 = true;
	int uiOrder = 0;
};

#endif // __ComponentButton_h__

