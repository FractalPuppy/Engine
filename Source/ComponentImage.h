#ifndef __ComponentImage_h__
#define __ComponentImage_h__

#include "Component.h"
#include "Math/float4.h"
#include <vector>

class ResourceTexture;

class ComponentImage : public Component
{
public:
	ComponentImage();
	ComponentImage(GameObject* gameobject);
	ComponentImage(const ComponentImage &copy);
	~ComponentImage();
	Component* Clone() const override;

	void Update() override;
	void UpdateTexturesList();
	ENGINE_API void UpdateTexture(std::string textureName);

	void DrawProperties() override;
	void Save(JSON_value* value) const override;
	void Load(JSON_value* value) override;

	ENGINE_API void SetMaskAmount(int maskAmount);
	int GetMaskAmount() const;

	bool IsMasked() const;

	float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);
	ResourceTexture* texture = nullptr;
	bool flipVertical = false;
	bool flipHorizontal = false;
	bool isMaskHorizontal = false;

	bool isHovered = false;
	bool isPressed = false;

	std::vector<std::string> textureFiles;
	bool enabled = true;
private:
	bool isMasked = false;
	int maskAmount = 100;

	bool updateImageList = false;
};

#endif // __ComponentImage_h__

