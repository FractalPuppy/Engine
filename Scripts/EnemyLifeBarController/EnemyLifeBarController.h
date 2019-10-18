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
class Text;
class Transform2D;

enum class EnemyLifeBarType
{
	NORMAL = 1,
	NORMAL_TEMPLE,
	ELITE_GRAVEYARD,
	ELITE_TEMPLE,
	BOSS,
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
	GameObject* enemyLife = nullptr;
	Text* enemyTypeName = nullptr;
	Transform2D* enemyTypeNamePosition = nullptr;
	ComponentImage* skull = nullptr;
	ComponentImage* lifeImage = nullptr;
	
	bool draw = false;

};

extern "C" EnemyLifeBarController_API Script* CreateScript();

#endif __EnemyLifeBarController_h__
