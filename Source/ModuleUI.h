#ifndef __ModuleUI_H__
#define __ModuleUI_H__

#include "Module.h"
#include <list>

struct Shader; 
struct Texture;

class ComponentCamera;
class ComponentImage;
class ComponentText;

class ModuleUI :
	public Module
{
public:
	ModuleUI();
	virtual ~ModuleUI();

	bool Init(JSON* json) override;
	update_status Update(float dt) override;
	update_status PostUpdate() override;
	bool CleanUp() override;

	void Draw(int currentWidth, int currentHeight);
public:
	std::list<ComponentImage*> images;
	std::list<ComponentText*> texts;
	int currentWidth;
	int currentHeight;
private:
	void RenderImage(const ComponentImage& componentImage, int currentWidth, int currentHeight);
private:
	const char* shaderFile = "UI";
	Shader* shader = nullptr;
	
	unsigned VAO = 0;
	unsigned VBO = 0;
	unsigned EBO = 0;
};

#endif // __ModuleUI_H__
