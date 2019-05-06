#ifndef __ModuleRender_h__
#define __ModuleRender_h__

#include "Module.h"
#include "Math/float3.h"
#include "MathGeoLib/include/Geometry/Frustum.h"
#include <unordered_set>

class ComponentCamera;
class JSON;
class ResourceSkybox;
class Viewport;
struct Shader;
class ComponentLight;
class ComponentRenderer;

class ModuleRender : public Module
{
public:
	ModuleRender();
	~ModuleRender();

	bool Init(JSON * config) override;
	bool Start() override;
	update_status PreUpdate() override;
	update_status Update(float dt) override;
	update_status PostUpdate() override;
	void SaveConfig(JSON* config) override;

	void Draw(const ComponentCamera& cam, int width, int height, bool isEditor = false) const;
	bool IsSceneViewFocused() const;
	bool IsSceneHovered() const;
	bool CleanUp() override;
	void OnResize();
	void ENGINE_API SetVsync(bool active);
	void DrawGUI() override;

	void GenBlockUniforms();
	void AddBlockUniforms(const Shader &s) const;

private:

	void DrawGizmos(const ComponentCamera &cam) const;
	void SetViewUniform(const ComponentCamera &camera) const;
	void SetProjectionUniform(const ComponentCamera &camera) const;
	void InitSDL();
	void InitOpenGL() const;
	void ComputeShadows();
	void ShadowVolumeDrawDebug();
	void BlitShadowTexture();

public:
	void* context = nullptr;

	bool msaa = true;
	int msaa_level = 8;
	bool msaa_lvl_changed = false;

	bool picker_debug = false;
	bool light_debug = true;
	bool kDTreeDebug = false;
	bool aabbTreeDebug = false;
	bool grid_debug = true;
	bool shadowDebug = false;
	bool boneDebug = false;
	bool useMainCameraFrustum = false;
	bool vsync = false;

	unsigned current_scale = 1;
	Viewport* viewGame = nullptr;
	Viewport* viewScene = nullptr;

	ComponentLight* directionalLight = nullptr;
	math::Frustum shadowsFrustum;
	unsigned shadowsTex = 0u;
	std::unordered_set<ComponentRenderer*> shadowCasters;


private:
	unsigned UBO = 0;
	bool depthTest = true;
	bool wireframe = false;
	int item_current = 0;//scale index

	ResourceSkybox* skybox = nullptr;

	//shadows stuff

	math::float3 lightPos;

	float shadowVolumeWidth;
	float shadowVolumeWidthHalf;
	float shadowVolumeHeight;
	float shadowVolumeHeightHalf;
	float shadowVolumeLength;


	unsigned shadowsFBO = 0u;

	bool shadowVolumeRendered = false;
	
	Shader* shadowsShader = nullptr;
};

#endif /* __ModuleRender_h__ */
