#ifndef __ComponentVolumetricLight_h__
#define __ComponentVolumetricLight_h__

#define VERT_AMOUNT 198
#define UV_AMOUNT 132
#define INDEX_AMOUNT 69
#define MAX_RADIUS 1000000.f

#include "Component.h"
#include "Math/float4.h"
#include <vector>

class GameObject;
class ResourceMesh;
class ComponentRenderer;

class ComponentVolumetricLight : public Component
{
	friend class GameObject;
public:
	ComponentVolumetricLight(GameObject* gameobject);
	ComponentVolumetricLight(const ComponentVolumetricLight &copy);
	~ComponentVolumetricLight();
	
	Component* Clone() const override;

	void Update() override;

	void DrawProperties() override;
	void Save(JSON_value* value) const override;
	void Load(JSON_value* value) override;
	void UpdateMesh();

	void Init();
	
	ComponentRenderer* renderer = nullptr;

private:

	float conePoints[VERT_AMOUNT];
	float coneUVs[UV_AMOUNT];
	int coneIndexes[INDEX_AMOUNT];
	float circle1Radius = 5.0f;
	float circle2Radius = 1.0f;
	float length = 10.0f;

	ResourceMesh* mesh = nullptr; //We keep a private mesh with the shape of the volumetric light
};

#endif // __ComponentVolumetricLight_h__

