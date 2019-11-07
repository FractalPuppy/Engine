#ifndef  __SpinToWinEnemyAI_h__
#define  __SpinToWinEnemyAI_h__

#include "BasicEnemyAIScript/BasicEnemyAIScript.h"
#include <vector>

#ifdef SpinToWinEnemyAI_EXPORTS
#define SpinToWinEnemyAI_API __declspec(dllexport)
#else
#define SpinToWinEnemyAI_API __declspec(dllimport)
#endif

class SpinToWinEnemyAI_API SpinToWinEnemyAI : public BasicEnemyAIScript
{
public:
	inline virtual SpinToWinEnemyAI* Clone() const
	{
		return new SpinToWinEnemyAI(*this);
	}
	void Start() override;
	void Update() override;
	void Expose(ImGuiContext* context) override;

	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;

	std::vector<GameObject*> spinBones;

	float spinDamage = 0.0f;

private:
	void RotateSpinBone();

	float boneRotationSpeed = 5.0f;
};

extern "C" SpinToWinEnemyAI_API Script* CreateScript();
#endif __DividingEnemyAI_h__
