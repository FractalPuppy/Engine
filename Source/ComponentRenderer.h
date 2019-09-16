#ifndef __ComponentRenderer_h__
#define __ComponentRenderer_h__

#include "Component.h"
#include "Math/float3.h"
#include "Math/float2.h"

class ResourceMesh;
class ResourceMaterial;
struct BindBone;

class ComponentRenderer :
	public Component
{
	friend class ResourceMaterial;

public:
	ComponentRenderer(GameObject* gameobject);
	ComponentRenderer(const ComponentRenderer& component);
	~ComponentRenderer();


	Component* Clone() const override;
	void DrawProperties() override;
	bool CleanUp() override;

	void Save(JSON_value* value) const override;
	void Load(JSON_value* value) override;

	ENGINE_API void SetMaterial(const char* materialName);
	ENGINE_API void ResetAnimation();

	ENGINE_API void SetMesh(const char* meshfile);
	void UpdateGameObject();
	void LinkBones();

	void DrawMesh(unsigned shaderProgram);

public:
	ResourceMesh* mesh = nullptr;
	ResourceMaterial* material = nullptr;
	bool castShadows = false;
	bool useAlpha = false;
	bool highlighted = false;
	bool dissolve = false;
	float dissolveAmount = 0.f;
	float borderAmount = 0.f;

	bool water = false;
	bool avoidSkinning = false;

	float waterAmplitude1 = 10.f;
	float waterFrequency1 = 10.f;
	float waterDecay1 = 1.f;
	math::float3 waterSource1 = math::float3::zero;

	float waterAmplitude2 = 10.f;
	float waterFrequency2 = 10.f;
	float waterDecay2 = 1.f;
	float distorsionSpeed = 0.001f;

	math::float2 uvScaler = math::float2::one;

	math::float3 waterSource2 = math::float3::zero;
	float waterSpeed = 2.f;
	math::float3 highlightColor = math::float3::one;
	bool animationEnded = false;

	bool isVolumetricLight = false;

private:

	void Update();
	void PickMaterial();

	void UpdateMeshesList();

	std::vector<std::string> guiMaterials;
	std::vector<std::string> guiMeshes;
	int xTiles = 1u;
	int yTiles = 1u;
	int f1Xpos = 0;
	int f1Ypos = 0;
	int f2Xpos = 0;
	int f2Ypos = 0;
	float fps = 24.f;
	float timer = 0.f;
	float frameMix = 0.f; 
	bool loop = false;
	math::float2 texSpeed = math::float2::zero;

	std::vector<BindBone> bindBones;
};

#endif __ComponentRenderer_h__
