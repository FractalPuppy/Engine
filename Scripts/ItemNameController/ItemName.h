#ifndef  __ItemName_h__
#define  __ItemName_h__

#include "Math/float2.h"

class ComponentTransform;
class GameObject;
class ItemPicker;

class ItemName
{
public:
	ComponentTransform* transform = nullptr;
	unsigned UID;
	math::float2 actualPosition = math::float2::zero;
	math::float2 distanceNormalized = math::float2::zero;
	float distanceDone = 0;
	bool show = false;
	bool hovered = false;
	ItemRarity rarity;
};

#endif

