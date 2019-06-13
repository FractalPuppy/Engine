#ifndef  __SkillTreeController_h__
#define  __SkillTreeController_h__

#include "BaseScript.h"
#include <vector>
#include "Skill.h"

#ifdef SkillTreeController_EXPORTS
#define SkillTreeController_API __declspec(dllexport)
#else
#define SkillTreeController_API __declspec(dllimport)
#endif

class GameObject;
class Text;
class JSON_value;
class Transform2D;

class SkillTreeController_API SkillTreeController : public Script
{
public:
	void Start() override;
	void Update() override;

	void AddSkillPoint();

	void Expose(ImGuiContext* context) override;
	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;

private:
	int skillPoints = 0;

	GameObject* skills = nullptr;
	GameObject* skillInfo = nullptr;
	Text* skillPointsLabel = nullptr;
	Transform2D* hoverTransform = nullptr;

	std::vector<GameObject*> skillUI;
	std::vector<Component*> connUI;

	std::vector<std::string> textureFiles;
	Skill skillList[13];
};

#endif __SkillTreeController_h__
