#include "DoorScript.h"

#include "Application.h"
#include "ModuleInput.h"
#include "ModuleTime.h"
#include "ModuleScene.h"

#include "ComponentAnimation.h"
#include "ComponentBoxTrigger.h"
#include "ComponentRenderer.h"
#include "ComponentTransform.h"
#include "ComponentAudioSource.h"
#include "GameObject.h"

#include "JSON.h"
#include <assert.h>
#include <string>
#include "imgui.h"
#include "Globals.h"
#include "debugdraw.h"

DoorScript_API Script* CreateScript()
{
	DoorScript* instance = new DoorScript;
	return instance;
}

void DoorScript::Start()
{
	player = App->scene->FindGameObjectByName(playerName.c_str());
	playerBbox = &App->scene->FindGameObjectByName(playerBboxName.c_str(), player)->bbox;

	anim = gameobject->GetComponent<ComponentAnimation>();
	if (anim == nullptr)
	{
		LOG("The GameObject %s has no Animation component attached \n", gameobject->name);
	}

	renderer1 = (App->scene->FindGameObjectByName(myBboxName1.c_str(), gameobject))->GetComponent<ComponentRenderer>();

	if (renderer1 != nullptr)
		myBbox1 = &App->scene->FindGameObjectByName(myBboxName1.c_str(), gameobject)->bbox;


	renderer2 = (App->scene->FindGameObjectByName(myBboxName2.c_str(), gameobject))->GetComponent<ComponentRenderer>();

	if (renderer2 != nullptr)
		myBbox2 = &App->scene->FindGameObjectByName(myBboxName2.c_str(), gameobject)->bbox;

	GameObject* GO = nullptr;

	GO = App->scene->FindGameObjectByName("open_gate");
	if (GO != nullptr)
	{
		open_gate = GO->GetComponent<ComponentAudioSource>();
		assert(open_gate != nullptr);
	}
	else
	{
		LOG("Warning: stepSound game object not found");
	}

}

void DoorScript::Update()
{
	if (!opened)
	{
		// Check collision with player
		if (myBbox1 != nullptr && myBbox1->Intersects(*playerBbox))
		{
			// Open door:
			anim->SendTriggerToStateMachine("Open");
			if (!activated)
			{
				open_gate->Play();
				activated = true;
			}
		}

		if (myBbox2 != nullptr && myBbox2->Intersects(*playerBbox))
		{
			// Open door:
			anim->SendTriggerToStateMachine("Open");
			if (!activated)
			{
				open_gate->Play();
				activated = true;
			}
		}
	}
}

void DoorScript::Expose(ImGuiContext * context)
{
	ImGui::SetCurrentContext(context);
}

void DoorScript::Serialize(JSON_value * json) const
{
}

void DoorScript::DeSerialize(JSON_value * json)
{
}

