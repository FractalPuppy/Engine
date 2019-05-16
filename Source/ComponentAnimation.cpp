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
	editorController = new AnimationController();
	controller = new AnimationController();
}

ComponentAnimation::ComponentAnimation(GameObject * gameobject) : Component(gameobject, ComponentType::Animation)
{
	editorController = new AnimationController();
	controller = new AnimationController();
}

ComponentAnimation::~ComponentAnimation()
{
	delete editorController;
	editorController = nullptr;

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

			CreateNewStateMachine();
		}
	
		ImGui::SameLine();

		ImGui::PushID("State Machine Combo");
		if (ImGui::BeginCombo("SM", stateMachine != nullptr ? stateMachine->GetName() : ""))
		{
			if (guiStateMachines.empty())
			{
				guiStateMachines = App->resManager->GetResourceNamesList(TYPE::STATEMACHINE, true);
			}
			for (unsigned i = 0u; i < guiStateMachines.size(); i++)
			{
				bool is_selected = (stateMachine != nullptr ? stateMachine->GetName() == guiStateMachines[i].c_str() : false);
				if (ImGui::Selectable(guiStateMachines[i].c_str(), is_selected))
				{
					if (stateMachine != nullptr)
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
			ImGui::InputText("SM name", newStMachineName, MAX_STATEMACHINE_NAME);
			ImGui::SameLine();
			if (ImGui::Button("Rename"))
			{
				stateMachine->Rename(newStMachineName);
			}

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

					//if the animation must finish we must set loop to false and viceversa!
					bool finish = stateMachine->GetClipMustFinish(j);
					if (ImGui::Checkbox("Must end", &finish))
					{
						if (!finish)
							stateMachine->SetClipMustFinish(j, false);
						else
						{
							stateMachine->SetClipMustFinish(j, true);
							stateMachine->SetClipLoop(j, false);
						}

						stateMachine->Save();
					}

					ImGui::SameLine();
					ImGui::PushItemWidth(60);
					float speed = stateMachine->GetClipSpeed(j);
					if (ImGui::DragFloat("Clip speed", &speed, 0.1f, 0.f, 10.f))
					{
						stateMachine->SetClipSpeed(j, speed);
						stateMachine->Save();
					}
					ImGui::PopItemWidth();

					bool clipLoop = stateMachine->GetClipLoop(j);
					if (ImGui::Checkbox("Loop", &clipLoop))
					{
						if (!clipLoop)
							stateMachine->SetClipLoop(j, false);
						else
						{
							stateMachine->SetClipLoop(j, true);
							stateMachine->SetClipMustFinish(j, false);
						}

						stateMachine->Save();
					}

					ImGui::SameLine();

					unsigned clipUID = stateMachine->GetClipResource(j);
					//IS THIS CORRECT=?=???
					ResourceAnimation* animation = (ResourceAnimation*)App->resManager->GetWithoutLoad(clipUID);
				
					if (animation != nullptr && ImGui::BeginCombo("", clipUID != 0u ? animation->GetName() : ""))
					{
						if (guiAnimations.empty())
						{
							guiAnimations = App->resManager->GetResourceNamesList(TYPE::ANIMATION, true);
						}
						for (int n = 0; n < guiAnimations.size(); n++)
						{
							bool is_selected = (clipUID != 0u ? HashString(animation->GetName()) == HashString(guiAnimations[n].c_str()) : false);
							if (ImGui::Selectable(guiAnimations[n].c_str(), is_selected))
							{
								unsigned animUID = ((ResourceAnimation*)App->resManager->GetByName(guiAnimations[n].c_str(), TYPE::ANIMATION))->GetUID();
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

void ComponentAnimation::CreateNewStateMachine()
{
	ResourceStateMachine* newStateMachine = new ResourceStateMachine(App->resManager->GenerateNewUID());

	newStateMachine->SetName("NewStateMachine");
	//Do not fear the while, accept it
	if (App->resManager->NameExists(newStateMachine->GetName(), TYPE::STATEMACHINE))
	{
		std::string newName = App->resManager->GetAvailableName(newStateMachine->GetName(), TYPE::STATEMACHINE);
		newStateMachine->Rename(newName.c_str());
	}
	newStateMachine->Save();
	RELEASE(newStateMachine);
}

void ComponentAnimation::SetAnimation(const char* animationFile)
{
	// Delete previous animation

	if (anim != nullptr)
		App->resManager->DeleteResource(anim->GetUID());

	if (animationFile != nullptr)
		anim = (ResourceAnimation*)App->resManager->GetByName(animationFile, TYPE::ANIMATION);

	return;
}

void ComponentAnimation::SetStateMachine(const char* stateMachineFile)
{
	// Delete previous stateMachine

	if (stateMachine != nullptr)
		App->resManager->DeleteResource(stateMachine->GetUID());

	if (stateMachineFile != nullptr)
	{
		stateMachine = (ResourceStateMachine*)App->resManager->GetByName(stateMachineFile, TYPE::STATEMACHINE);
		strcpy(newStMachineName, stateMachine->GetName());
	}
}

void ComponentAnimation::SendTriggerToStateMachine(const char* trigger)
{
	if (stateMachine != nullptr)
	{	
		if (controller->CanSwitch())
		{
			unsigned prevNode = stateMachine->GetDefaultNode();
			float blend = 0.f;

			stateMachine->ReceiveTrigger(HashString(trigger), blend);
			if (prevNode != stateMachine->GetDefaultNode())
			{
				SetIndexChannels(gameobject, GetAnimFromStateMachine());
				PlayNextNode(blend);
			}
		}
	}
}

ResourceAnimation* ComponentAnimation::GetAnimFromStateMachine()
{
	unsigned nodeIndex = stateMachine->GetDefaultNode();
	HashString clipName = stateMachine->GetNodeClip(nodeIndex);
	unsigned clipIndex = stateMachine->FindClip(clipName);
	unsigned animUID = stateMachine->GetClipResource(clipIndex);
	ResourceAnimation*  resAnim = (ResourceAnimation*)(App->resManager->Get(animUID));
	return resAnim;
}

bool ComponentAnimation::GetLoopFromStateMachine()
{
	unsigned nodeIndex = stateMachine->GetDefaultNode();
	HashString clipName = stateMachine->GetNodeClip(nodeIndex);
	return stateMachine->GetClipLoop(stateMachine->FindClip(clipName));
}

float ComponentAnimation::GetSpeedFromStateMachine()
{
	unsigned nodeIndex = stateMachine->GetDefaultNode();
	HashString clipName = stateMachine->GetNodeClip(nodeIndex);
	return stateMachine->GetClipSpeed(stateMachine->FindClip(clipName));
}

bool ComponentAnimation::GetMustFinishFromStateMachine()
{
	unsigned nodeIndex = stateMachine->GetDefaultNode();
	HashString clipName = stateMachine->GetNodeClip(nodeIndex);
	return stateMachine->GetClipMustFinish(stateMachine->FindClip(clipName));
}

void ComponentAnimation::PlayNextNode(float blend)
{
	if(stateMachine != nullptr)
		controller->PlayNextNode(GetAnimFromStateMachine(),GetLoopFromStateMachine(), GetMustFinishFromStateMachine(),
			GetSpeedFromStateMachine(), blend);
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
	if (App->time->gameState == GameState::RUN) //Game run time exclusive
	{
		if (stateMachine != nullptr && stateMachine->GetClipsSize() > 0u && stateMachine->GetNodesSize() > 0u)
		{
			if (!channelsSetted)
			{
				SetIndexChannels(gameobject, GetAnimFromStateMachine());
				channelsSetted = true;
			}
			controller->Update(App->time->gameDeltaTime);

			if (gameobject != nullptr)
			{
				UpdateGO(gameobject);
			}
		}
	}
	else if(App->time->gameState == GameState::STOP)// Editor exclusive
	{
		if (isPlaying)
		{
			if (!channelsSetted)
			{
				SetIndexChannels(gameobject, editorController->current->anim);
				channelsSetted = true;
			}

			editorController->Update(App->time->realDeltaTime);

			if (gameobject != nullptr)
			{
				EditorUpdateGO(gameobject);
			}
		}
	}
}

void ComponentAnimation::OnPlay()
{
	if (stateMachine != nullptr && stateMachine->GetClipsSize() > 0u && stateMachine->GetNodesSize() > 0u)
	{
		controller->Play(GetAnimFromStateMachine(), GetLoopFromStateMachine(),
			GetMustFinishFromStateMachine(), GetSpeedFromStateMachine());
	}
}

void ComponentAnimation::UpdateGO(GameObject* go)
{
	PROFILE;
	
	if (go->animationIndexChannel != 999u)
	{
		float3 position;
		Quat rotation;

		if (controller->GetTransform(go->animationIndexChannel, position, rotation))
		{
			go->transform->SetPosition(position);
			go->transform->SetRotation(rotation);
		}

		gameobject->movedFlag = true;
	}

	for (std::list<GameObject*>::iterator it = go->children.begin(); it != go->children.end(); ++it)
	{
		UpdateGO(*it);
	}
}

void ComponentAnimation::EditorUpdateGO(GameObject* go)
{
	PROFILE;
	float3 position;
	Quat rotation;

	if (go->animationIndexChannel != 999u)
	{
		if (editorController->GetTransform(go->animationIndexChannel, position, rotation))
		{
			go->transform->SetPosition(position);
			go->transform->SetRotation(rotation);
		}

		gameobject->movedFlag = true;
	}

	for (std::list<GameObject*>::iterator it = go->children.begin(); it != go->children.end(); ++it)
	{
		EditorUpdateGO(*it);
	}
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

void ComponentAnimation::SetIndexChannels(GameObject* GO, ResourceAnimation* anim)
{
	GO->animationIndexChannel = 999u;
	GO->animationIndexChannel = anim->GetIndexChannel(GO->name.c_str());

	for (const auto& child : GO->children)
	{
		SetIndexChannels(child, anim);
	}
}