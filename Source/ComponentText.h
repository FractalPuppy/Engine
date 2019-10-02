#ifndef __ComponentText_h__
#define __ComponentText_h__

#include "Component.h"

#include "Math/float4.h"
#include "Math/float2.h"

class Text :
	public Component
{
public:
	Text();
	Text(GameObject* gameobject);
	Text(const Text &copy);
	~Text();
	Component* Clone() const override;

	void DrawProperties() override;
	void Save(JSON_value* value) const override;
	void Load(JSON_value* value) override;
public:
	float fontSize = 64;
	std::string text = "Fractal Text";
	std::string font = "";
	math::float4 color = math::float4(1.0f, 1.0f, 1.0f, 0.0f);
	math::float4 colorHovered = math::float4(1.0f, 1.0f, 1.0f, 0.0f);
	math::float2 offset = math::float2::zero;
	math::float2 scaleOffset = math::float2::one;
	bool isHovered = false;
	bool isTextWrapped = false;
	float wrapWidth = 100;
	float interlineDistance = 24;
	int uiOrder = 0;
};

#endif
