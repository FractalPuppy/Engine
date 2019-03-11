#ifndef __GUICreator_h__
#define __GUICreator_h__

class GameObject;
class GUICreator
{
public:
	~GUICreator();
	static void CreateElements(GameObject* go);

private:
	GUICreator();
};

#endif __GUICreator_h__