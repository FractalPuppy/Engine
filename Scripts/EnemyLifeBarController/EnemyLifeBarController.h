#ifndef  __EnemyLifeBarController_h__
#define  __EnemyLifeBarController_h__

#include "BaseScript.h"

#ifdef EnemyLifeBarController_EXPORTS
#define EnemyLifeBarController_API __declspec(dllexport)
#else
#define EnemyLifeBarController_API __declspec(dllimport)
#endif

class GameObject;
class ComponentImage;

enum class EnemyLifeBarType
{
	NORMAL = 0,
	HARD,
	BOSS
};

class EnemyLifeBarController_API EnemyLifeBarController : public Script
{

public:
	void Start() override;
	void Update() override;

	void SetLifeBar(int maxHP, int actualHP, EnemyLifeBarType type, std::string name);

private:
	GameObject* lifeBackground = nullptr;
	GameObject* hPbar = nullptr;
	GameObject* enemyTypeName = nullptr;
	GameObject* boneRight = nullptr;
	GameObject* boneLeft = nullptr;
	GameObject* skull = nullptr;

	ComponentImage* lifeImage = nullptr;
};

#endif __EnemyLifeBarController_h__
