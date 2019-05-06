#ifndef  __CameraController_h__
#define  __CameraController_h__

#include "BaseScript.h"
#include "Math/float3.h"

class GameObject;

#ifdef CameraController_EXPORTS
#define CameraController_API __declspec(dllexport)
#else
#define CameraController_API __declspec(dllimport)
#endif

class CameraController_API CameraController : public Script
{
	void Start() override;
	void Update() override;

private:
	GameObject* player;
	math::float3 offset;
};

#endif __CameraController_h__
