#ifndef __ComponentLight_h__
#define __ComponentLight_h__

#include "Component.h"
#include "Math/float3.h"
#include "MathGeoLib/include/Geometry/Sphere.h"
#define LIGHTTYPES 3

enum class LightType
{
	DIRECTIONAL = 0,
	POINT,
	SPOT
};

class GameObject;

class ComponentLight :
	public Component
{
public:
	ComponentLight(GameObject* gameobject);
	ComponentLight(const ComponentLight& component);
	~ComponentLight();
	ComponentLight* Clone() const override;

	void ResetValues();

	void Update() override;
	void DrawProperties() override;
	void DrawDebugLight() const;

	void Load(JSON_value* value) override;
	void Save(JSON_value* value) const override;
	
	void Paste() override;
	void Reset() override;

	void CalculateGuizmos();
	void DrawDebug() const;

public:
	LightType lightType = LightType::POINT;

	math::float3 position = math::float3::zero; //Captured in transform
	math::float3 direction = math::float3::one;
	math::float3 color = math::float3::one;
	
	float inner = 20.f;
	float outer = 25.f;
	float intensity = 1.f;
	float range = 100.f;

	Sphere pointSphere;
private:

	float spotDistance = .0f;	
	float spotEndRadius = .0f;		
};

#endif __ComponentLight_h__