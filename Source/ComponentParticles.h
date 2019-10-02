#ifndef __ComponentParticles_h__
#define __ComponentParticles_h__

#define MAX_DISTANCE 10000000.f
#define MAX_RATE 10000000.f

#include "Component.h"
#include <string>
#include "Math/float3.h"
#include "Math/float2.h"
#include "Math/float4x4.h"
#include "Math/Quat.h"
#include "Math/float4.h"

#include <queue>
#include <vector>
#include "ComponentTransform.h"
#include "ModuleRender.h"


class ResourceTexture;
class ParticleModule;

struct Particle
{
	float speed = 1.f;

	float axisRotation = 0.f;
	math::float3 position = math::float3::zero;
	math::float4x4 global = math::float4x4::zero;
	math::float3 direction = math::float3::unitY;
	math::float2 size = math::float2::zero;

	float totalLifetime = .0f;
	float lifeTimer = totalLifetime;
	math::float4 color;
	
};


class ComponentParticles :
	public Component
{
	friend class ModuleParticles;

public:

	enum EmisorType
	{
		QUAD = 0,
		SPHERE,
		CONE
	};

	ComponentParticles(GameObject* gameobject);
	ComponentParticles(const ComponentParticles& component);
	~ComponentParticles();
	
	Component* Clone() const override;
	void DrawProperties() override;
	bool CleanUp() override;

	void Update(float dt, const math::float3& camPos);

	void Save(JSON_value* value) const override;
	void Load(JSON_value* value) override;

	ResourceTexture* texture = nullptr;

	std::list<Particle*> particles;
	std::queue<Particle*> particlePool;

private:
	void Reset();
	void alternateEmisor(int i);
	void DrawDebugEmisor();
	float3 randomSpherePoint(float3 center);

public:
	float PlayTime = 1.f;
	float lastActive = 0.f;

private:

	std::vector<std::string> textureFiles;

	std::vector<ParticleModule*> modules;

	int xTiles = 1;
	int yTiles = 1;
	float fps = 24.f;

	float timer = 0.f;
	float frameMix = 0.f; 
	int f1Xpos;
	int f1Ypos;
	int f2Xpos;
	int f2Ypos;

	math::float2 lifetime = math::float2::one;
	math::float2 speed = math::float2::one * App->renderer->current_scale;
	float rate = 10.f;
	float rateTimer = 1.f / rate;
	int maxParticles = 50;
	float intensity = 1.0f;

	math::float2 axisRotation = math::float2::zero;
	math::float2 particleMinSize = math::float2(1.f * App->renderer->current_scale, 1.f * App->renderer->current_scale);
	math::float2 particleMaxSize = math::float2(1.f * App->renderer->current_scale, 1.f * App->renderer->current_scale);
	math::float2 quadEmitterSize = math::float2(10.f * App->renderer->current_scale);
	math::float4 particleColor = math::float4::one;
	math::float3 pDir = math::float3(-1.f, 0.f, 0.f);

	bool directionNoise = false;
	int directionNoiseProbability = 5;
	int directionNoiseTotalProbability = 500;

	EmisorType actualEmisor = EmisorType::QUAD;
	bool quadCheck = true;
	bool sphereCheck = false;
	bool coneCheck = false;
	//cone properties
	float baseRadius = 3.f;
	float apexRadius = 1.5f;
	std::vector<bool*> emisorsCheck = { &quadCheck,&sphereCheck, &coneCheck};

	bool sizeOTCheck = false;
	bool colorOTCheck = false;

	bool billboarded = true;
	bool aligned = false;
	bool localEmitter = false;
	math::float3 lookAtTarget = math::float3::unitY;

};


#endif __ComponentParticles_h__