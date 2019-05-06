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

					int startTime = stateMachine->GetClipStartFrame(j);
					if (ImGui::InputInt("Start Time", &startTime))
					{
						stateMachine->SetClipStartFrame(j, startTime);
					}
					//ImGui::SameLine();
					int endTime = stateMachine->GetClipEndFrame(j);
					if (ImGui::InputInt("End Time", &endTime))
					{
						stateMachine->SetClipEndFrame(j, endTime);
					}
					//ImGui::PopItemWidth();

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

void ComponentAnimation::SendTriggerToStateMachine(const char* trigger)
{
	if (stateMachine != nullptr)
	{	
		if (controller->CanSwitch())
		{
			unsigned prevNode = stateMachine->GetDefaultNode();
			unsigned blend = 0u;

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
	ResourceAnimation*  resAnim = (ResourceAnimation*)(App->resManager->GetWithoutLoad(animUID));
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

int ComponentAnimation::GetStartFrameFromStateMachine()
{
	unsigned nodeIndex = stateMachine->GetDefaultNode();
	HashString clipName = stateMachine->GetNodeClip(nodeIndex);
	return stateMachine->GetClipStartFrame(stateMachine->FindClip(clipName));
}

int ComponentAnimation::GetEndFrameFromStateMachine()
{
	unsigned nodeIndex = stateMachine->GetDefaultNode();
	HashString clipName = stateMachine->GetNodeClip(nodeIndex);
	return stateMachine->GetClipEndFrame(stateMachine->FindClip(clipName));
}

bool ComponentAnimation::GetMustFinishFromStateMachine()
{
	unsigned nodeIndex = stateMachine->GetDefaultNode();
	HashString clipName = stateMachine->GetNodeClip(nodeIndex);
	return stateMachine->GetClipMustFinish(stateMachine->FindClip(clipName));
}

void ComponentAnimation::PlayNextNode(unsigned blend)
{
	if(stateMachine != nullptr)
		controller->PlayNextNode(GetAnimFromStateMachine(),GetLoopFromStateMachine(), GetMustFinishFromStateMachine(),
			GetSpeedFromStateMachine(), GetStartFrameFromStateMachine(), GetEndFrameFromStateMachine(), blend);
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
	if (stateMachine != nullptr && stateMachine->GetClipsSize() > 0u && stateMachine->GetNodesSize() > 0u)
	{
	
		if (App->time->gameState == GameState::RUN)
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
	else if (isPlaying)
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

void ComponentAnimation::OnPlay()
{
	if (stateMachine != nullptr && stateMachine->GetClipsSize() > 0u && stateMachine->GetNodesSize() > 0u)
	{
		controller->Play(GetAnimFromStateMachine(), GetLoopFromStateMachine(),
			GetMustFinishFromStateMachine(), GetSpeedFromStateMachine(), GetStartFrameFromStateMachine(), GetEndFrameFromStateMachine());
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

	if (editorController->GetTransform(go->animationIndexChannel, position, rotation))
	{
		go->transform->SetPosition(position);
		go->transform->SetRotation(rotation);
	}

	gameobject->movedFlag = true;

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
	GO->animationIndexChannel = anim->GetIndexChannel(GO->name.c_str());

	for (const auto& child : GO->children)
	{
		SetIndexChannels(child, anim);
	}
}