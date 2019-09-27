#include "BombExplosionScript.h"

BombExplosionScript_API Script* CreateScript()
{
	BombExplosionScript* instance = new BombExplosionScript;
	return instance;
}

void BombExplosionScript::Awake()
{
}

void BombExplosionScript::Start()
{
}

void BombExplosionScript::Update()
{
	switch (currentState)
	{
	case ExplosionState::None:
		break;
	case ExplosionState::Appear:
		break;
	case ExplosionState::Grow:
		break;
	case ExplosionState::Explode:
		break;
	}
}

void BombExplosionScript::OnTriggerEnter(GameObject * go)
{
}

void BombExplosionScript::Expose(ImGuiContext * context)
{
}

void BombExplosionScript::Serialize(JSON_value * json) const
{
}

void BombExplosionScript::DeSerialize(JSON_value * json)
{
}
