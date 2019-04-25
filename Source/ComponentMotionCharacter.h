#ifndef __ComponentMotionCharacter_h__
#define __ComponentMotionCharacter_h__

#include "Component.h"

#include "Math/float4x4.h"
#include "Math/float3.h"
#include "Math/Quat.h"

class ComponentMotionCharacter : public Component
{
public:
	float MaxLinearSpeed;
	float MaxAngularSpeed;
private:
	enum CinematicMovement {
		No_Forces,
		Linear_Speed,
		Angular_Speed
	};
	enum MovementType {
		Forward,
		Backward,
		Strafe_Left,
		Strafe_Right
	};
	math::float4x4 local;
	math::float4x4 global;
	math::Quat& rotation;
public:
	void Update() override;
	void Move(MovementType movType, CinematicMovement cinMovement, float movementSpeed);
};
#endif