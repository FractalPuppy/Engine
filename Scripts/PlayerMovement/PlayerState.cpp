#include "PlayerState.h"
#include "PlayerMovement.h"
#include "Math/float2.h"
#include "Math/float3.h"
#include "MathGeoLib/Geometry/GeometryAll.h"
#include "ComponentTransform.h"
#include "GameObject.h"

#include "Application.h"
#include "ModuleTime.h"

PlayerState::PlayerState(PlayerMovement * PM, const char * trigger, 
	math::float3 boxSize, float minTime, float maxTime) : 
	player(PM), trigger(trigger), boxSize(boxSize), minTime(minTime), maxTime(maxTime)
{
}

PlayerState::~PlayerState()
{
}

void PlayerState::UpdateTimer()
{
	timer += player->App->time->fullGameDeltaTime;
}

void PlayerState::lerpCalculations(const math::float3& direction, math::float3 playerFront, const math::float3& nextPointPosition)
{
	//math::float3 playerFront = -player->gameobject->transform->front;
	float angleResult = direction.AngleBetweenNorm(playerFront.Normalized());

	//block of code to check how much the direction has to be changed to look at the next point
	{
		//0.26 rad is 15 deg, if player looking 15 deg or less off, we just look at the walking direction
		if (angleResult < 0.26f && angleResult > -0.26f)
		{
			player->gameobject->transform->LookAt(nextPointPosition);
			currentLerping = 0.00;
		}
		//case the difference is bigger, the player lerps
		else
		{
			//getting the values in 2D avoids so much trobule
			//we get the starting and final direction in 2D
			math::float2 startingDirection = math::float2(playerFront.Normalized().x, playerFront.Normalized().z);
			math::float2 finalDirection = math::float2(direction.x, direction.z);

			//calculate the current direction using the Lerp function
			math::float2 directionToLook2D = startingDirection.Lerp(finalDirection, currentLerping);
			//set things together to calculate the Y value using the Dot product (perpendicular of x and z)
			math::float3 directionToLookX(directionToLook2D.x, 0, 0);
			math::float3 directionToLookZ(0, 0, directionToLook2D.y);
			//get the desired direction
			math::float3 directionToLook3D = math::float3(RadToDeg(directionToLook2D.x), directionToLookZ.Dot(directionToLookX), RadToDeg(directionToLook2D.y));

			//look at the point
			player->gameobject->transform->LookAt(player->gameobject->transform->GetGlobalPosition() + directionToLook3D);
			//increment lerping value for the next iteration taking into consideration both the delta time and the current angle
			//we take into consideration the angle because if the player is to rotate 180 degrees, it takes a little too long
			currentLerping += lerpingIncrement * player->App->time->gameDeltaTime * angleResult;
		}
	}
}
