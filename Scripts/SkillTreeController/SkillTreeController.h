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

#define NUM_SKILLS 11

class SkillTreeController_API SkillTreeController : public Script
{
public:
	void Start() override;
	void UnlockNextLevel(int i);
	void Update() override;

	void AddSkillPoint();
	std::vector<Skill> GetActiveSkills();

	void Expose(ImGuiContext* context) override;
	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;

	void SaveSkillTree();

	inline virtual SkillTreeController* Clone() const
	{
		return new SkillTreeController(*this);
	}
private:

	int skillPoints = 0;

	GameObject* skills = nullptr;
	GameObject* skillInfo = nullptr;
	Text* skillPointsLabel = nullptr;
	Transform2D* hoverTransform = nullptr;

	std::vector<GameObject*> skillUI;
	std::vector<Component*> connUI;

	std::vector<std::string> textureFiles;
	Skill skillList[NUM_SKILLS];

	int initialSkillPoints = 0;
};

extern "C" SkillTreeController_API Script* CreateScript();

#endif __SkillTreeController_h__
