#ifndef  __DividingEnemyAI_h__
#define  __DividingEnemyAI_h__

#include "BasicEnemyAIScript/BasicEnemyAIScript.h"
class EnemyStateMultiply;

#ifdef DividingEnemyAI_EXPORTS
#define DividingEnemyAI_API __declspec(dllexport)
#else
#define DividingEnemyAI_API __declspec(dllimport)
#endif

class DividingEnemyAI_API DividingEnemyAI : public BasicEnemyAIScript
{
public:
	inline virtual DividingEnemyAI* Clone() const
	{
		return new DividingEnemyAI(*this);
	}
	void Start() override;
	void Update() override;

public:
	EnemyStateMultiply* multiply = nullptr;
};

extern "C" DividingEnemyAI_API Script* CreateScript();
#endif __DividingEnemyAI_h__
