#ifndef __ModuleUI_H__
#define __ModuleUI_H__

#define MASK_DIVISIONS 20

#include "Module.h"
#include <list>

struct Shader; 
struct Texture;

class ComponentCamera;
class ComponentImage;
class Text;

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

	void DrawGUI() override;

	void Draw(int currentWidth, int currentHeight);
public:

	int currentWidth;
	int currentHeight;
	bool showUIinSceneViewport = false;


private:
	void GenerateVAO(unsigned& vao, float quadVertices[16]);
	void RenderImage(const ComponentImage& componentImage, int currentWidth, int currentHeight);
private:
	const char* shaderFile = "UI";
	Shader* shader = nullptr;
	
	unsigned VAO = 0;
	unsigned VAO_FV = 0;
	unsigned VAO_FH = 0;
	unsigned VBO = 0;
	unsigned EBO = 0;

	float mask[MASK_DIVISIONS];
};

#endif // __ModuleUI_H__
