#ifndef __ModuleUI_H__
#define __ModuleUI_H__

#define MASK_DIVISIONS 20

#include "Module.h"
#include <list>

struct Shader; 
struct Texture;

class Component;
class ComponentCamera;
class ComponentImage;
class Button;
class Text;

struct RenderOrdering
{
	RenderOrdering(int order, unsigned count, Component* component) : order(order), count(count), component(component) {}
	const bool operator<(const RenderOrdering &other) const { return (order > other.order) || (order == other.order && count > other.count); };

	int order = 0;
	unsigned count = 0U;
	Component* component = nullptr;
};

class ModuleUI :
	public Module
{
public:
	ModuleUI();
	virtual ~ModuleUI();

	bool Init(JSON* config) override;
	void SaveConfig(JSON* config) override;
	update_status PreUpdate() override { uiHoveredMouse1 = false; uiHoveredMouse3 = false; topButton = nullptr; return update_status::UPDATE_CONTINUE; }
	update_status Update(float dt) override;
	update_status PostUpdate() override;
	bool CleanUp() override;

	void DrawGUI() override;

	void Draw(int currentWidth, int currentHeight);
	inline bool UIHovered(bool checkMouse1 = true, bool checkMouse3 = true) { return (uiHoveredMouse1 && checkMouse1) || (uiHoveredMouse3 && checkMouse3); }
	ENGINE_API inline bool IsHover() { return isHover; }
	ENGINE_API inline void SetIsItemHover(bool isItemHover) { this->isItemHover = isItemHover; }
	void SetButtonHover(const Button* button);
	inline const Button* GetButtonHover() { return topButton; }

public:
	int currentWidth;
	int currentHeight;
	bool showUIinSceneViewport = false;

	std::string gameStandarCursor = "Glow.cur";
private:
	void GenerateVAO(unsigned& vao, float quadVertices[16]);
	void RenderImage(const ComponentImage& componentImage, int currentWidth, int currentHeight);
	void ManageUiHoveredCursorIcon(bool isHovered);

private:
	const char* shaderFile = "UI";
	Shader* shader = nullptr;
	
	unsigned VAO = 0;
	unsigned VAO_FV = 0;
	unsigned VAO_FH = 0;
	unsigned VBO = 0;
	unsigned EBO = 0;

	float mask[MASK_DIVISIONS];
	const Button* topButton = nullptr;

	std::string uiCursor = "GhostGlow.cur";
	bool isHover = false;
	bool isItemHover = false;
	bool changeHoverCursorIcon = true;
	bool changeNotHoverCursorIcon = true;

public:
	bool uiHoveredMouse1 = false;
	bool uiHoveredMouse3 = false;

};

#endif // __ModuleUI_H__
