#ifndef  __Skill_h__
#define  __Skill_h__

#include <string>

class ResourceTexture;

class Skill
{

public:
	int id;
	std::string name;
	std::string description;
	ResourceTexture* spriteActive = nullptr;
	ResourceTexture* spriteInactive;
	bool locked = true;
	bool available = false;
	int maxLevels = 5;
	int currentLevel = 0;
	int nextSkill = -1;
	int connection = -1;
	int mana = 0;
};

#endif
