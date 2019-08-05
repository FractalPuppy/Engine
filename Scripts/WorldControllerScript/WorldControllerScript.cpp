#include "WorldControllerScript.h"

#include "Application.h"
#include "GameObject.h"

#include "Globals.h"

#include <memory>

WorldControllerScript_API Script* CreateScript()
{
	WorldControllerScript* instance = new WorldControllerScript;
	return instance;
}

WorldControllerScript::WorldControllerScript()
{
}

WorldControllerScript::~WorldControllerScript()
{
}

void WorldControllerScript::setPlayer(const GameObject* player)
{
	this->player = player;
}

void WorldControllerScript::addEnemy(const GameObject* enemy)
{
}