#ifndef __ComponentTransform_h__
#define __ComponentTransform_h__

#include "Component.h"
#include "Math/float3.h"
#include "Math/Quat.h"
#include "Math/float4x4.h"

class ComponentTransform : public Component
{
public:
	ComponentTransform(GameObject* gameobject, const float4x4 &transform = float4x4::identity);
	ComponentTransform(const ComponentTransform& component);
	~ComponentTransform();

	Component* Clone() const override;
	void AddTransform(const float4x4 &transform);
	void DrawProperties() override;

	void UpdateTransform();
	void SetLocalToWorld();
	void SetWorldToLocal(const float4x4 & newparentGlobalMatrix);
	void SetGlobalTransform(const float4x4 & newglobal, const float4x4 &parentglobal);

	float3 GetGlobalPosition();

	void Save(JSON_value *value) const override;
	void Load(const JSON_value &value) override;

private:
	void RotationToEuler();
	void UpdateOldTransform();

public:
	float3 position = float3::zero;
	Quat rotation = Quat::identity;
	float3 eulerRotation = float3::zero;
	float3 scale = float3::zero;
	
	float4x4 local = float4x4::identity;
	float4x4 global = float4x4::identity;

private:
	float3 old_position = float3::zero;
	float3 old_euler = float3::zero;
	float3 old_scale = float3::zero;
};

#endif __ComponentTransform_h__

