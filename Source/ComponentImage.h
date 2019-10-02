#ifndef __ComponentImage_h__
#define __ComponentImage_h__

#include "Component.h"
#include "Math/float4.h"
#include <vector>
#include "opencv2/opencv.hpp"

class ResourceTexture;
class Transform2D;

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
	ENGINE_API float PlayVideo(); //Returns duration
	ENGINE_API void StopVideo();

	float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);
	ResourceTexture* texture = nullptr;
	bool flipVertical = false;
	bool flipHorizontal = false;
	bool isMaskHorizontal = false;

	bool isHovered = false;
	bool isPressed = false;

	bool showHoverDetectInEditor = true;
	bool hoverDetectionMouse1 = true;
	bool hoverDetectionMouse3 = true;

	std::vector<std::string> textureFiles;
	
	bool enabled = true;
	int uiOrder = 0;

	bool videoPlaying = false;

	unsigned videoTex = 0u;
	bool videoFinished = false;
	bool loop = false;

private:
	bool isMasked = false;
	int maskAmount = 100;

	bool updateImageList = false;

	std::string videoPath = "Intro.mp4";
	cv::Mat frame;
	cv::VideoCapture cap;
	float frameTime;
	float fps = 24.0f;
	float frameTimer;
};

#endif // __ComponentImage_h__

