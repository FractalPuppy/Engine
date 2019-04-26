#ifndef __ComponentTransform_h__
#define __ComponentTransform_h__

#include "Component.h"
#include "Math/float3.h"
#include "Math/Quat.h"
#include "Math/float4x4.h"

class ComponentTransform : public Component
{
public:
	ComponentTransform(GameObject* gameobject, const math::float4x4& transform = math::float4x4::identity);
	ComponentTransform(const ComponentTransform& component);
	~ComponentTransform();

	Component* Clone() const override;
	void AddTransform(const math::float4x4 &transform);
	void DrawProperties() override;

	void MultiSelectionTransform(float4x4 &difference); //checks if multi transform is required & do it

	void UpdateTransform();
	void UpdateOldTransform();
	void UpdateGlobalTransform();

	void SetLocalToWorld();
	void SetWorldToLocal(const math::float4x4& newparentGlobalMatrix);
	void SetGlobalTransform(const math::float4x4& newglobal, const math::float4x4& parentglobal);
	void SetLocalTransform(const math::float4x4& newLocal, const math::float4x4& parentGlobal); //only for importing purposes / if used later watch the trees

	ENGINE_API void SetPosition(const math::float3& position);
	ENGINE_API void SetRotation(const math::Quat & newRotation);
	ENGINE_API math::Quat GetRotation();
	ENGINE_API math::float3 GetPosition();

	ENGINE_API math::float3 GetGlobalPosition();

	ENGINE_API void LookAt(const math::float3& target);
	ENGINE_API void Align(const math::float3& target);

	void Save(JSON_value* value) const override;
	void Load(JSON_value* value) override;

	//void Copy() override;
	void Paste() override;
	void Reset() override;

private:
	void RotationToEuler();

public:
	math::float3 position = math::float3::zero;
	math::Quat rotation = math::Quat::identity;
	math::float3 eulerRotation = math::float3::zero;
	math::float3 scale = math::float3::zero;
	math::float4x4 local = math::float4x4::identity;
	math::float4x4 animatedLocal = math::float4x4::identity;
	math::float4x4 global = math::float4x4::identity;

	math::float3 up = math::float3::zero;
	math::float3 right = math::float3::zero;
	math::float3 front = math::float3::zero;

private:
	math::float3 old_position = math::float3::zero;
	math::float3 old_euler = math::float3::zero;
	math::float3 old_scale = math::float3::zero;
};

#endif __ComponentTransform_h__
