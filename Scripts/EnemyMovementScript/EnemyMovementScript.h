#ifndef  __EnemyMovementScript_h__
#define  __EnemyMovementScript_h__

class GameObject;
class JSON_value;
class PlayerMovement;
class AABB;
struct ImGuiContext;

#include "BaseScript.h"
#include "Math/float3.h"
#include "Math/Quat.h"
#include "Math/float4x4.h"
#include "Geometry/AABB.h"


#ifdef EnemyMovementScript_EXPORTS
#define EnemyMovementScript_API __declspec(dllexport)
#else
#define EnemyMovementScript_API __declspec(dllimport)
#endif

class EnemyMovementScript_API EnemyMovementScript : public Script
{
public:
	void Expose(ImGuiContext* context) override;

	void Start() override;
	void Update() override;
	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;
public:
	bool stop = false;
private:
	GameObject* GetGameObjectRecursiveByName(GameObject* gameObject, const char* name);
private:
	float speed = 100.0f;
	float maxPlayerDistance = 200000.0f;
	std::string playerName = "Player";
	GameObject* player = nullptr;
	PlayerMovement* playerScript = nullptr;

	math::AABB *myBbox = nullptr;
	math::AABB *targetBbox = nullptr;

};
#endif __EnemyMovementScript_h__
