#include "ScaleOverTimeScript.h"

ScaleOverTimeScript_API Script* CreateScript()
{
	ScaleOverTimeScript* instance = new ScaleOverTimeScript;
	return instance;
}

void ScaleOverTimeScript::Start()
{
}

void ScaleOverTimeScript::Update()
{
}

void ScaleOverTimeScript::Expose(ImGuiContext * context)
{
}

void ScaleOverTimeScript::Serialize(JSON_value * json) const
{
}

void ScaleOverTimeScript::DeSerialize(JSON_value * json)
{
}
