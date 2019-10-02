#ifndef  __CameraController_h__
#define  __CameraController_h__

#include "BaseScript.h"
#include "Math/float3.h"
#include "Algorithm/Random/LCG.h"
#include "Math/Quat.h"

class GameObject;

#ifdef CameraController_EXPORTS
#define CameraController_API __declspec(dllexport)
#else
#define CameraController_API __declspec(dllimport)
#endif

class CameraController_API CameraController : public Script
{
public:
	void Start() override;
	void Update() override;
	inline virtual CameraController* Clone() const
	{
		return new CameraController(*this);
	}

	/** @param fadeInTime value between [0,1].
	@@ param fadeOutTime value between [0,1].
	Represents percentage of shake time that fadeIn ends and fadeOut starts**/
	void Shake(float duration, float intensity = 5.0f, float fadeInTime = 0.2f, float fadeOutTime = 0.8f, bool linearFade = false);

private:
	void ShakeCamera(math::float3& position);

private:
	GameObject* player;
	math::float3 offset;

	bool isShaking = false;
	float shakeDuration = 0.0f;
	float shakeIntensity = 0.0f;
	float shakeTimer = 0.0f;
	float shakeFadeInTime = 0.0f;
	float shakeFadeOutTime = 0.0f;
	float roll = 0.0f;
	bool shakelinearFade = false;

	Quat originalRotation = Quat::identity;

	LCG rand;
};

extern "C" CameraController_API Script* CreateScript();

#endif __CameraController_h__