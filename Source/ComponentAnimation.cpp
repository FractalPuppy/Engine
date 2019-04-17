#include "Application.h"

#include "ModuleResourceManager.h"
#include "ModuleFileSystem.h"
#include "ModuleTime.h"

#include "GameObject.h"
#include "Resource.h"
#include "ResourceAnimation.h"
#include "AnimationController.h"
#include "ComponentAnimation.h"
#include "ComponentTransform.h"

#include "Imgui/include/imgui.h"
#include "JSON.h"
#include "Math/Quat.h"
#include "Math/float3.h"
#include "Brofiler.h"

ComponentAnimation::ComponentAnimation() : Component(nullptr, ComponentType::Animation)
{
	controller = new AnimationController();
}


ComponentAnimation::~ComponentAnimation()
{
	controller = nullptr;
	anim = nullptr;
	gameobject->isBoneRoot = false;
	RELEASE_ARRAY(animName);
}


void ComponentAnimation::DrawProperties()
{
	if (ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Animation");
		ImGui::PushID("Animation Combo");
		if (ImGui::BeginCombo("", anim != nullptr ? anim->name.c_str() : ""))
		{
			if (guiAnimations.empty())
			{
				guiAnimations = App->resManager->GetAnimationsNamesList(true);
			}
			for (int n = 0; n < guiAnimations.size(); n++)
			{
				bool is_selected = (anim != nullptr ? anim->name == guiAnimations[n] : false);
				if (ImGui::Selectable(guiAnimations[n].c_str(), is_selected))
				{
					if (anim == nullptr || anim->name != guiAnimations[n])
					{
						SetAnimation(guiAnimations[n].c_str());
						PlayAnimation(100u);
					}
				}
				if (is_selected)
				{
					ImGui::SetItemDefaultFocus();
				}
				
			}
			ImGui::EndCombo();
		}
		else
		{
			guiAnimations.clear();
		}
		ImGui::PopID();
	}
}

void ComponentAnimation::ResetResource()
{
	anim->DeleteFromMemory();
}

void ComponentAnimation::SetAnimation(const char* animationFile)
{
	// Delete previous animation

	if (anim != nullptr)
		App->resManager->DeleteResource(anim->GetUID());

	if (animationFile != nullptr)
		anim = (ResourceAnimation*)App->resManager->GetAnimationByName(animationFile);

	return;
}

void ComponentAnimation::Update()
{
	PROFILE;
	if (anim != nullptr)
	{
		if (!channelsSetted)
		{
			SetIndexChannels(gameobject);
			channelsSetted = true;
		}

		if (App->time->gameState == GameState::RUN)
		{
			controller->Update(App->time->gameDeltaTime);

			if (gameobject != nullptr)
			{
				UpdateGO(gameobject);
			}
		}
		else if (isPlaying)
		{
			controller->Update(App->time->realDeltaTime);

			if (gameobject != nullptr)
			{
				UpdateGO(gameobject);
			}
		}
	}
}

void ComponentAnimation::UpdateGO(GameObject* go)
{
	PROFILE;
	float3 position;
	Quat rotation;

	if (controller->GetTransform(go->animationIndexChannel, position, rotation))
	{
		go->transform->SetPosition(position);
		go->transform->SetRotation(rotation);
	}

	gameobject->movedFlag = true;

	for (std::list<GameObject*>::iterator it = go->children.begin(); it != go->children.end(); ++it)
	{
		UpdateGO(*it);
	}
}

void ComponentAnimation::PlayAnimation(unsigned blend)
{
	controller->Play(anim, true, blend);
}

Component* ComponentAnimation::Clone() const
{
	return new ComponentAnimation(*this);
}

ComponentAnimation::ComponentAnimation(GameObject * gameobject) : Component(gameobject, ComponentType::Animation)
{
	controller = new AnimationController();
	PlayAnimation(100u);
}

ComponentAnimation::ComponentAnimation(const ComponentAnimation& component) : Component(component)
{
	anim = (ResourceAnimation*)App->resManager->Get(component.anim->GetUID());
}


bool ComponentAnimation::CleanUp()
{
	if (anim != nullptr)
	{
		App->resManager->DeleteResource(anim->GetUID());
	}
	return true;
}

void ComponentAnimation::Save(JSON_value* value) const
{
	Component::Save(value);
	value->AddUint("animUID", (anim != nullptr) ? anim->GetUID() : 0u);
}

void ComponentAnimation::Load(JSON_value* value)
{
	Component::Load(value);
	unsigned uid = value->GetUint("animUID");

	anim = (ResourceAnimation*)App->resManager->Get(uid);
}

void ComponentAnimation::SetIndexChannels(GameObject* GO)
{
	GO->animationIndexChannel = anim->GetIndexChannel(GO->name.c_str());

	for (const auto& child : GO->children)
	{
		SetIndexChannels(child);
	}

}