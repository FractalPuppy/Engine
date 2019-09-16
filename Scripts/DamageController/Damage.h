#ifndef  __Damage_h__
#define  __Damage_h__

#include "Math/float2.h"

enum class DamageType
{
	NONE = 0,
	HEALING,
	NORMAL,
	CRITICAL,
	POLLOMETER,
	RECIEVED,
	MANA
};

class ComponentTransform;

class Damage
{

public:
	int damage;
	ComponentTransform*  transform = nullptr;
	math::float2 actualPosition = math::float2::zero;
	math::float2 distanceNormalized = math::float2::zero;
	float distanceDone = 0;
	DamageType type = DamageType::NONE;
	bool show = false;
};

#endif