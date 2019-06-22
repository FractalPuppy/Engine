#ifndef __ComponentButton_h__
#define __ComponentButton_h__

#include "Component.h"

class ComponentImage;
class Text;
class Transform2D;
class ResourceTexture;

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
	bool isKeyDown = false;
	bool isKeyUp = false;

	bool hoverDetectionMouse1 = true;
	bool hoverDetectionMouse3 = true;
};

#endif // __ComponentButton_h__

