#include "Application.h"

#include "ModuleResourceManager.h"
#include "ModuleFileSystem.h"
#include "ModuleTime.h"

#include "GameObject.h"
#include "Resource.h"
#include "ResourceAnimation.h"
#include "ResourceStateMachine.h"
#include "AnimationController.h"
#include "ComponentAnimation.h"
#include "ComponentTransform.h"

#include "Globals.h"
#include <stack>
#include "HashString.h"
#include "Imgui/include/imgui.h"
#include "JSON.h"
#include "Math/Quat.h"
#include "Math/float3.h"
#include "Brofiler.h"

ComponentAnimation::ComponentAnimation() : Component(nullptr, ComponentType::Animation)
{

	controller = new AnimationController();
}

ComponentAnimation::ComponentAnimation(GameObject * gameobject) : Component(gameobject, ComponentType::Animation)
{
	controller = new AnimationController();
}

ComponentAnimation::~ComponentAnimation()
{
	delete controller;
	controller = nullptr;
	if (context)
	{
		ax::NodeEditor::DestroyEditor(context);
		context = nullptr;
	}
	anim = nullptr;
	stateMachine = nullptr;
	gameobject->isBoneRoot = false;
	RELEASE_ARRAY(animName);
}


void ComponentAnimation::DrawProperties()
{
	ImGui::PushID(this);
	if (ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool removed = Component::DrawComponentState();
		if (removed)
		{
			ImGui::PopID();
			return;
		}

		if (ImGui::Button("New State Machine"))
		{
			if (stateMachine != nullptr)
				stateMachine->Save();

			stateMachine = (ResourceStateMachine*)App->resManager->CreateNewResource(TYPE::STATEMACHINE);
		}
	
		ImGui::SameLine();

		ImGui::PushID("State Machine Combo");
		if (ImGui::BeginCombo("SM", stateMachine != nullptr ? stateMachine->name.c_str() : ""))
		{
			if (guiStateMachines.empty())
			{
				guiStateMachines = App->resManager->GetSMNamesList(true);
			}
			for (unsigned i = 0u; i < guiStateMachines.size(); i++)
			{
				bool is_selected = (stateMachine != nullptr ? stateMachine->name == guiStateMachines[i] : false);
				if (ImGui::Selectable(guiStateMachines[i].c_str(), is_selected))
				{
					if(stateMachine != nullptr)
						stateMachine->Save();

					SetStateMachine(guiStateMachines[i].c_str());
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
			guiStateMachines.clear();
		}
		ImGui::PopID();

		if (stateMachine != nullptr)
		{
			//Here we should have the name of the stateMachine
			char* smName = new char[MAX_CLIP_NAME];
			strcpy(smName, stateMachine->name.c_str());
			ImGui::InputText("SM name", smName, MAX_CLIP_NAME);
			stateMachine->name = smName;

			if (ImGui::Button("AddClip"))
			{
				stateMachine->AddClip(HashString("Clippity clip"), 0u, true);
				stateMachine->Save();
			}
			ImGui::SameLine();
			if (ImGui::Button("Save SM"))
			{
				stateMachine->Save();
			}
			ImGui::SameLine();
			if (ImGui::Button("Delete SM"))
			{
				deletePopup = !deletePopup;
			}
			if (deletePopup)
			{
				ImGui::Text("Are you sure? this can't be undone"); ImGui::SameLine();
				if (ImGui::Button("Yes"))
				{
					stateMachineDelete = true;
				} ImGui::SameLine();
				if (ImGui::Button("Cancel"))
				{
					deletePopup = false;
				}
			}

			if (!stateMachine->isClipsEmpty())
			{
				ImGui::Separator();
				for (unsigned j = 0u; j < stateMachine->GetClipsSize(); ++j)
				{
					ImGui::PushID(j);
					char* clipName = new char[MAX_CLIP_NAME];
					strcpy(clipName, stateMachine->GetClipName(j).C_str());
					ImGui::InputText("Clip name", clipName, MAX_CLIP_NAME);

					//Find all the nodes that use this clip and change them
					for (unsigned k = 0u; k < stateMachine->GetNodesSize(); k++)
					{
						if (stateMachine->GetNodeClip(k) == stateMachine->GetClipName(j))
							stateMachine->SetNodeClip(k, HashString(clipName));
					}
					stateMachine->SetClipName(j, HashString(clipName));

					bool clipLoop = stateMachine->GetClipLoop(j);
					if (ImGui::Checkbox("Loop", &clipLoop))
					{
						if (!clipLoop)
							stateMachine->SetClipLoop(j, false);
						else
							stateMachine->SetClipLoop(j, true);

						stateMachine->Save();
					}

					ImGui::SameLine();

					unsigned clipUID = stateMachine->GetClipResource(j);
					//IS THIS CORRECT=?=???
					ResourceAnimation* animation = (ResourceAnimation*)App->resManager->GetWithoutLoad(clipUID);
					if (ImGui::BeginCombo("", clipUID != 0u ? animation->name.c_str() : ""))
					{
						if (guiAnimations.empty())
						{
							guiAnimations = App->resManager->GetAnimationsNamesList(true);
						}
						for (int n = 0; n < guiAnimations.size(); n++)
						{
							bool is_selected = (clipUID != 0u ? animation->name == guiAnimations[n] : false);
							if (ImGui::Selectable(guiAnimations[n].c_str(), is_selected))
							{
								unsigned animUID = ((ResourceAnimation*)App->resManager->GetAnimationByName(guiAnimations[n].c_str()))->GetUID();
								stateMachine->SetClipResource(j, animUID);
								stateMachine->Save();
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

					if (ImGui::Button("Remove Clip"))
					{
						clipIndexToRemove = j;
						clipRemove = true;
					}

					ImGui::Separator();

					ImGui::PopID();
				}
			}

			if (clipRemove)
			{
				stateMachine->RemoveClip(clipIndexToRemove);
				clipRemove = false;
			}
		}
		
	}
	ImGui::PopID();
	if (stateMachineDelete)
	{
		stateMachine->Delete();
		stateMachineDelete = false;
	}
}

void ComponentAnimation::ResetResource()
{
	anim->DeleteFromMemory();
	stateMachine->DeleteFromMemory();
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

void ComponentAnimation::SetStateMachine(const char * stateMachineFile)
{
	// Delete previous stateMachine

	if (stateMachine != nullptr)
		App->resManager->DeleteResource(stateMachine->GetUID());

	if (stateMachineFile != nullptr)
		stateMachine = (ResourceStateMachine*)App->resManager->GetSMByName(stateMachineFile);
}

void ComponentAnimation::SendTriggerToStateMachine(HashString trigger)
{
	if (stateMachine != nullptr)
	{	
		unsigned blend = 0u;
		stateMachine->ReceiveTrigger(trigger, blend);
		PlayNextNode(blend);
	}
}

ResourceAnimation * ComponentAnimation::GetAnimFromStateMachine()
{
	unsigned nodeIndex = stateMachine->GetDefaultNode();
	HashString clipName = stateMachine->GetNodeClip(nodeIndex);
	unsigned clipIndex = stateMachine->FindClip(clipName);
	unsigned animUID = stateMachine->GetClipResource(clipIndex);
	ResourceAnimation*  resAnim = (ResourceAnimation*)(App->resManager->GetWithoutLoad(animUID));
	return resAnim;
}

bool ComponentAnimation::GetLoopFromStateMachine()
{
	unsigned nodeIndex = stateMachine->GetDefaultNode();
	HashString clipName = stateMachine->GetNodeClip(nodeIndex);
	return stateMachine->GetClipLoop(clipName);
}

void ComponentAnimation::PlayNextNode(unsigned blend)
{
	controller->PlayNextNode(GetAnimFromStateMachine(),GetLoopFromStateMachine(), blend);
}

ComponentAnimation::EditorContext* ComponentAnimation::GetEditorContext()
{
	if (context == nullptr)
	{
		ax::NodeEditor::Config cfg;
		cfg.SettingsFile = "simple.json";
		context = ax::NodeEditor::CreateEditor(&cfg);
	}
	return context;
}

void ComponentAnimation::Update()
{
	PROFILE;
	if (stateMachine != nullptr)
	{
	
		if (App->time->gameState == GameState::RUN)
		{
			if (!channelsSetted)
			{
				SetIndexChannels(gameobject);
				channelsSetted = true;
			}

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

void ComponentAnimation::OnPlay()
{
	controller->Play(GetAnimFromStateMachine(), GetLoopFromStateMachine());
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

//old
void ComponentAnimation::PlayAnimation(unsigned blend)
{
	controller->Play(anim, true);
}

Component* ComponentAnimation::Clone() const
{
	return new ComponentAnimation(*this);
}


ComponentAnimation::ComponentAnimation(const ComponentAnimation& component) : Component(component)
{
	if(component.stateMachine != nullptr)
		stateMachine = (ResourceStateMachine*)App->resManager->Get(component.stateMachine->GetUID());
}


bool ComponentAnimation::CleanUp()
{
	if (anim != nullptr)
	{
		App->resManager->DeleteResource(anim->GetUID());
	}
	if (stateMachine != nullptr)
	{
		App->resManager->DeleteResource(stateMachine->GetUID());
	}
	return true;
}

void ComponentAnimation::Save(JSON_value* value) const
{
	Component::Save(value);
	value->AddUint("animUID", (anim != nullptr) ? anim->GetUID() : 0u);
	value->AddUint("stateMachineUID", (stateMachine != nullptr) ? stateMachine->GetUID() : 0u);
}

void ComponentAnimation::Load(JSON_value* value)
{
	Component::Load(value);

	unsigned animUID = value->GetUint("animUID");
	anim = (ResourceAnimation*)App->resManager->Get(animUID);

	unsigned stateMachineUID = value->GetUint("stateMachineUID");
	stateMachine = (ResourceStateMachine*)App->resManager->Get(stateMachineUID);
}

void ComponentAnimation::SetIndexChannels(GameObject* GO)
{
	GO->animationIndexChannel = anim->GetIndexChannel(GO->name.c_str());

	for (const auto& child : GO->children)
	{
		SetIndexChannels(child);
	}
}