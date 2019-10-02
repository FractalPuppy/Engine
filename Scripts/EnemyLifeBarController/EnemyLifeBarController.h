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
	NORMAL = 1,
	HARD,
	BOSS
};

class EnemyLifeBarController_API EnemyLifeBarController : public Script
{

public:
	void Start() override;
	void Update() override;

	void SetLifeBar(int maxHP, int actualHP, EnemyLifeBarType type, std::string name);
	
	inline virtual EnemyLifeBarController* Clone() const
	{
		return new EnemyLifeBarController(*this);
	}

private:
	GameObject* lifeBackground = nullptr;
	GameObject* hPbar = nullptr;
	GameObject* enemyTypeName = nullptr;
	GameObject* boneRight = nullptr;
	GameObject* boneLeft = nullptr;
	GameObject* skull = nullptr;
	GameObject* enemyLife = nullptr;
	bool draw = false;

	ComponentImage* lifeImage = nullptr;
};

extern "C" EnemyLifeBarController_API Script* CreateScript();

#endif __EnemyLifeBarController_h__
