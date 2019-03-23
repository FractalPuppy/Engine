#ifndef __ComponentTransform2D_h__
#define __ComponentTransform2D_h__

#include "Component.h"
#include "Math/float2.h"

class ComponentTransform2D : public Component
{
public:
	ComponentTransform2D(GameObject* gameobject);
	ComponentTransform2D(const ComponentTransform2D& component);
	~ComponentTransform2D();

	Component* Clone() const override;
	void DrawProperties() override;

	void Save(JSON_value* value) const override;
	void Load(JSON_value* value) override;

public:

	math::float2 position = math::float2::zero;
	math::float2 size = math::float2(100.0f, 100.0f);

};

#endif

