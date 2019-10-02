#include "Application.h"

#include "ModuleScene.h"
#include "ModuleResourceManager.h"

#include "GameObject.h"

#include "ResourceAnimation.h"
#include "ModuleTime.h"
#include "ModuleFileSystem.h"

#include "ComponentAnimation.h"
#include "ComponentTransform.h"
#include "PanelAnimation.h"
#include "imgui.h"
#include "Globals.h"

#define eventPopup "Event"
#define clipPopup "Clipping"

PanelAnimation::PanelAnimation()
{
}


PanelAnimation::~PanelAnimation()
{
}

void PanelAnimation::Draw()
{
	if (!ImGui::Begin("Animation", &enabled))
	{
		ImGui::End();
		return;
	}
	if (App->scene->selected != nullptr && App->scene->selected->isBoneRoot && (ComponentAnimation*)(App->scene->selected->GetComponentOld(ComponentType::Animation)))
	{
		ComponentAnimation* compAnim = ((ComponentAnimation*)(App->scene->selected->GetComponentOld(ComponentType::Animation)));

		ImGui::Text("GAMEOBJECT"); ImGui::SameLine();
		ImGui::Text("CURRENT ANIM"); ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - ImGui::CalcTextSize("ANIMATION").x / 2);
		ImGui::Text("ANIMATION");
		ImGui::Separator();
		// GameObject
		ImGui::Text(App->scene->selected->name.c_str()); ImGui::SameLine();

		// Current Anim
		ImGui::SetCursorPosX(ImGui::CalcTextSize("  GAMEOBJECT  ").x);



		ImGui::PushItemWidth(100);

		if (ImGui::BeginCombo("##label", anim != nullptr ? anim->GetName() : ""))
		{
			if (guiAnimations.empty())
			{
				guiAnimations = App->resManager->GetResourceNamesList(TYPE::ANIMATION, true);
			}
			for (int n = 0; n < guiAnimations.size(); n++)
			{
				bool is_selected = true;
				if (ImGui::Selectable(guiAnimations[n].c_str(), is_selected))
				{
					// Delete previous animation
					if (anim != nullptr)
						App->resManager->DeleteResource(anim->GetUID());

					anim = ((ResourceAnimation*)App->resManager->GetByName(guiAnimations[n].c_str(), TYPE::ANIMATION));
					compAnim->editorController->PlayEditor(anim);
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
		ImGui::PopItemWidth();

		if (anim == nullptr)
		{
			ImGui::End();
			return;
		}

		ImGui::SameLine();
		if (ImGui::Button("Save Animation"))
		{
			anim->SaveNewAnimation();
		}

		// Animation
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - ImGui::CalcTextSize("FRAMES").x / 2);
		ImGui::Text("FRAMES");

		if (NULL == compAnim->editorController->current && NULL != anim)
		{
			compAnim->editorController->PlayEditor(anim);
		}

		ImGui::PushItemWidth(100);
		ImGui::DragFloat("Speed", &compAnim->editorController->current->speed, 0.05f); ImGui::SameLine();
		ImGui::PopItemWidth();

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 5.7f);

		if (ImGui::SliderInt("##label", &anim->currentFrame, 0, anim->duration))
		{
			compAnim->SetIndexChannels(App->scene->selected, anim);

			UpdateGameObjectAnimation(App->scene->selected, anim);

			if (!isCliping)
			{
				compAnim->editorController->current->time = anim->currentFrame / anim->framesPerSecond;
			}
			else if (isCliping && anim->currentFrame < minFrame)
			{
				anim->currentFrame = minFrame + 1;
			}
		}

		ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::GetWindowWidth() / 6);

		if (ImGui::Button("Clip", ImVec2(60, 23)))
		{
			if (!isCliping && anim->currentFrame + 1 <= anim->duration)
			{
				minFrame = anim->currentFrame;
				anim->currentFrame++;
				isCliping = true;
			}
			else if (isCliping && !compAnim->isPlaying)
			{
				newClip = true;
			}
		}
		if (isCliping)
		{
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(60, 23)) && !compAnim->isPlaying)
			{
				isCliping = false;
				anim->currentFrame = minFrame;
				compAnim->editorController->ResetClipping();
			}
		}

		if (newClip)
		{
			NewClipPopUp(anim, minFrame, maxFrame);
			guiAnimations.clear();
			isCliping = false;
		}

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - 43);
		if (ImGui::Button("<<", ImVec2(23, 23)))
		{
			compAnim->SetIndexChannels(App->scene->selected, anim);

			if (isCliping && anim->currentFrame - 1 > minFrame)
			{
				anim->currentFrame--;

				UpdateGameObjectAnimation(App->scene->selected, anim);
				compAnim->editorController->current->time = anim->currentFrame / anim->framesPerSecond;
			}
			else if (!isCliping && anim->currentFrame > 0)
			{
				anim->currentFrame--;
				UpdateGameObjectAnimation(App->scene->selected, anim);
				compAnim->editorController->current->time = anim->currentFrame / anim->framesPerSecond;
			}

		}
		ImGui::SameLine();

		if (compAnim->isPlaying)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, { 0.5f, 0.3f, 0.3f, 0.7f });

			if (ImGui::ArrowButton("play", ImGuiDir_Right)) // PAUSE
			{
				if (isCliping && compAnim->editorController->current->loop)
				{
					anim->currentFrame = maxFrame;
					UpdateGameObjectAnimation(App->scene->selected, anim);
				}

				if (compAnim->isPlaying)
					compAnim->isPlaying = false;
			}

			ImGui::PopStyleColor();
		}
		else
		{
			if (ImGui::ArrowButton("play", ImGuiDir_Right)) // PLAY
			{
				if (isCliping)
				{
					compAnim->editorController->current->minTime = minFrame / anim->framesPerSecond;
					compAnim->editorController->current->maxTime = maxFrame / anim->framesPerSecond;
				}

				compAnim->isPlaying = true;
				compAnim->editorController->SetNextEvent();
			}
		}

		ImGui::SameLine();
		if (ImGui::Button(">>", ImVec2(23, 23)))
		{
			compAnim->SetIndexChannels(App->scene->selected, anim);

			if (anim->currentFrame < anim->duration)
			{
				anim->currentFrame++;
				UpdateGameObjectAnimation(App->scene->selected, anim);
				compAnim->editorController->current->time = anim->currentFrame / anim->framesPerSecond;
			}
		}

		if (isCliping)
		{
			if (!compAnim->isPlaying)
			{
				maxFrame = anim->currentFrame;
			}

			ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::GetWindowWidth() / 6);
			ImGui::PushItemWidth(60);
			ImGui::DragInt("Frame Start", &minFrame, 1.0f, 0, maxFrame - 1); ImGui::PopItemWidth();
			ImGui::SameLine(); ImGui::Text("Frame Start");

			ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::GetWindowWidth() / 6);
			ImGui::PushItemWidth(60);
			ImGui::DragInt("Frame End", &maxFrame, 1.0f, minFrame + 1, anim->duration); ImGui::PopItemWidth();
			ImGui::SameLine(); ImGui::Text("Frame End");
		}

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 20);
		ImGui::Text("EVENTS"); ImGui::SameLine();
		if (ImGui::Button("+", ImVec2(20, 20)))
		{
			newEvent = true;
		}

		if (newEvent)
		{
			NewEventPopUp(compAnim);
		}

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 20);
		ImGui::Text("ID"); ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 14);
		ImGui::Text("FRAME"); ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 9);
		ImGui::Text("NAME OF EVENT");

		for (std::vector<Event*>::iterator it = anim->events.begin(); it != anim->events.end(); ++it)
		{
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 20);
			ImGui::Text(std::to_string((*it)->key).c_str()); ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 14);
			ImGui::Text(std::to_string((int)((*it)->frame)).c_str()); ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 9);
			ImGui::Text((*it)->name.c_str());
			ImGui::PushID((*it)->key); ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 5);
			if (ImGui::Button("Go"))
			{
				anim->currentFrame = (*it)->frame;
				UpdateGameObjectAnimation(App->scene->selected, anim);
			}
			ImGui::SameLine();
			if (ImGui::Button("X"))
			{
				keyToDelete = (*it)->key;
				isDeletingEvent = true;
			}
			ImGui::PopID();
		}

		if (isDeletingEvent && keyToDelete != -1)
		{
			anim->DeleteEvent(keyToDelete);

			keyToDelete = -1;
			isDeletingEvent = false;
		}

	}
	ImGui::End();
}


void PanelAnimation::UpdateGameObjectAnimation(GameObject * go, ResourceAnimation* anim)
{

	if (go->animationIndexChannel != 999u)
	{
		go->transform->SetPosition(anim->GetPosition(go->animationIndexChannel, anim->currentFrame));
		go->transform->SetRotation(anim->GetRotation(go->animationIndexChannel, anim->currentFrame));
	}

	for (auto child : go->children)
	{
		UpdateGameObjectAnimation(child, anim);
	}
}

void PanelAnimation::CreateAnimationFromClip(ResourceAnimation* anim, int minFrame, int maxFrame, char* newName)
{
	ResourceAnimation* newAnim = new ResourceAnimation(App->resManager->GenerateNewUID());

	for (int i = 0; i < anim->numberOfChannels; ++i)
	{
		Channel* newChannel = new Channel();

		newChannel->channelName = anim->channels[i]->channelName;
		if (anim->channels[i]->numPositionKeys == 1)
		{
			newChannel->numPositionKeys = 1;
		}
		else
		{
			newChannel->numPositionKeys = maxFrame - minFrame + 1;
		}

		if (anim->channels[i]->numRotationKeys == 1)
		{
			newChannel->numRotationKeys = 1;
		}
		else
		{
			newChannel->numRotationKeys = maxFrame - minFrame + 1;
		}

		if (anim->channels[i]->numPositionKeys == 1)
		{
			math::float3 position = anim->channels[i]->positionSamples[0];
			newChannel->positionSamples.push_back(position);
		}
		else
		{
			for (int j = 0; j < newChannel->numPositionKeys; ++j)
			{
				math::float3 position = anim->channels[i]->positionSamples[minFrame + j];
				newChannel->positionSamples.push_back(position);
			}
		}
	
		if (anim->channels[i]->numRotationKeys == 1)
		{
			math::Quat rotation = anim->channels[i]->rotationSamples[0];
			newChannel->rotationSamples.push_back(rotation);
		}
		else
		{
			for (int j = 0; j < newChannel->numRotationKeys; ++j)
			{
				math::Quat rotation = anim->channels[i]->rotationSamples[minFrame + j];
				newChannel->rotationSamples.push_back(rotation);
			}
		}

		newAnim->channels.push_back(newChannel);
	}

	newAnim->duration = maxFrame - minFrame; //frames
	newAnim->framesPerSecond = anim->framesPerSecond;
	newAnim->numberFrames = maxFrame - minFrame;
	newAnim->numberOfChannels = anim->numberOfChannels;
	newAnim->durationInSeconds = (maxFrame - minFrame) / anim->framesPerSecond;

	//Do not fear the while, accept it
	std::string finalName = App->resManager->GetAvailableName(newName, TYPE::ANIMATION);
	newAnim->Rename(finalName.c_str());

	newAnim->SaveNewAnimation();

	RELEASE(newAnim);
}

void PanelAnimation::NewClipPopUp(ResourceAnimation* anim, int minFrame, int maxFrame)
{
	if (!ImGui::IsPopupOpen(clipPopup))
	{
		ImGui::OpenPopup(clipPopup);
	}
	if (ImGui::BeginPopupModal(clipPopup, NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Create new clip:");
		ImGui::Separator();
		ImGui::InputText("NewClip", newName, 64);

		if (ImGui::Button("Save", ImVec2(120, 0)))
		{
			CreateAnimationFromClip(anim, minFrame, maxFrame, newName);
			newClip = false;

			strcpy(newName, "New Name");
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			newClip = false;
			strcpy(newName, "New Name");
		}

		ImGui::EndPopup();
	}
}

void PanelAnimation::NewEventPopUp(ComponentAnimation* compAnim)
{
	if (!ImGui::IsPopupOpen(eventPopup))
	{
		ImGui::OpenPopup(eventPopup);
	}
	if (ImGui::BeginPopupModal(eventPopup, NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Create new event:");
		ImGui::Separator();
		ImGui::InputText("NewEvent", newName, 64);

		if (ImGui::Button("Save", ImVec2(120, 0)))
		{
		
			anim->AddEvent(std::string(newName));
			newEvent = false;

			strcpy(newName, "New Name");

		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			newEvent = false;
			strcpy(newName, "New Name");
		}

		ImGui::EndPopup();
	}
}
