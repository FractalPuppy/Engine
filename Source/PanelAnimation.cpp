#include "Application.h"

#include "ModuleScene.h"
#include "ModuleResourceManager.h"

#include "GameObject.h"

#include "ResourceAnimation.h"

#include "ComponentAnimation.h"
#include "ComponentTransform.h"
#include "PanelAnimation.h"
#include "imgui.h"
#include "Globals.h"

PanelAnimation::PanelAnimation()
{
}


PanelAnimation::~PanelAnimation()
{
}

void PanelAnimation::Draw()
{
	//if (!ImGui::Begin("Animation", &enabled))
	//{
	//	ImGui::End();
	//	return;
	//}
	//if (App->scene->selected != nullptr && App->scene->selected->isBoneRoot && (ComponentAnimation*)(App->scene->selected->GetComponent(ComponentType::Animation)) && ((ComponentAnimation*)(App->scene->selected->GetComponent(ComponentType::Animation)))->anim != nullptr)
	//{
	//	ResourceAnimation* anim = ((ComponentAnimation*)(App->scene->selected->GetComponent(ComponentType::Animation)))->anim;
	//	ComponentAnimation* compAnim = ((ComponentAnimation*)(App->scene->selected->GetComponent(ComponentType::Animation)));

	//	ImGui::Text("GAMEOBJECT"); ImGui::SameLine();
	//	ImGui::Text("CURRENT ANIM"); ImGui::SameLine();
	//	ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - ImGui::CalcTextSize("ANIMATION").x / 2);
	//	ImGui::Text("ANIMATION");
	//	ImGui::Separator();
	//	// GameObject
	//	ImGui::Text(App->scene->selected->name.c_str()); ImGui::SameLine();

	//	// Current Anim
	//	ImGui::SetCursorPosX(ImGui::CalcTextSize("  GAMEOBJECT  ").x);
	//	ImGui::Text(anim->name.c_str());

	//	// Animation
	//	ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - ImGui::CalcTextSize("FRAMES").x / 2);
	//	ImGui::Text("FRAMES");

	//	ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 5.7f);

	//	if (ImGui::SliderInt("##label", &anim->currentFrame, 0, anim->duration))
	//	{
	//		UpdateGameObjectAnimation(App->scene->selected, anim);
	//		
	//		if (!isCliping)
	//		{
	//			compAnim->controller->current->time = anim->currentFrame / anim->framesPerSecond;
	//		}
	//		else if (isCliping && anim->currentFrame < minFrame)
	//		{
	//			anim->currentFrame = minFrame + 1;
	//		}
	//	}

	//	ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::GetWindowWidth() / 6);

	//	if (ImGui::Button("Clip", ImVec2(60, 23)))
	//	{
	//		if (!isCliping && anim->currentFrame + 1 <= anim->duration)
	//		{
	//			minFrame = anim->currentFrame;
	//			anim->currentFrame++;
	//			isCliping = true;
	//		}
	//		else if (isCliping && !compAnim->isPlaying)
	//		{
	//			CreateAnimationFromClip(anim, minFrame, maxFrame);
	//			isCliping = false;
	//			compAnim->controller->ResetClipping();
	//		}
	//	}

	//	if (isCliping)
	//	{
	//		ImGui::SameLine();
	//		if (ImGui::Button("Cancel") && !compAnim->isPlaying)
	//		{
	//			isCliping = false;
	//			compAnim->controller->ResetClipping();
	//		}
	//	}

	//	ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - 43);
	//	if (ImGui::Button("<<", ImVec2(23, 23)))
	//	{
	//		if (isCliping && anim->currentFrame - 1 > minFrame)
	//		{
	//			anim->currentFrame--;
	//			UpdateGameObjectAnimation(App->scene->selected, anim);
	//			compAnim->controller->current->time = anim->currentFrame / anim->framesPerSecond;
	//		}
	//		else if (!isCliping && anim->currentFrame > 0)
	//		{
	//			anim->currentFrame--;
	//			UpdateGameObjectAnimation(App->scene->selected, anim);
	//			compAnim->controller->current->time = anim->currentFrame / anim->framesPerSecond;
	//		}

	//	}
	//	ImGui::SameLine();

	//	if (compAnim->isPlaying)
	//	{
	//		ImGui::PushStyleColor(ImGuiCol_Button, { 0.5f, 0.3f, 0.3f, 0.7f });

	//		if (ImGui::ArrowButton("play", ImGuiDir_Right)) // PAUSE
	//		{				
	//			if (isCliping && compAnim->controller->current->loop)
	//			{
	//				anim->currentFrame = maxFrame;
	//				UpdateGameObjectAnimation(App->scene->selected, anim);
	//			}

	//			if (compAnim->isPlaying)
	//				compAnim->isPlaying = false;
	//		}

	//		ImGui::PopStyleColor();
	//	}
	//	else
	//	{
	//		if (ImGui::ArrowButton("play", ImGuiDir_Right)) // PLAY
	//		{
	//			if (isCliping)
	//			{
	//				compAnim->controller->current->minTime = minFrame / anim->framesPerSecond;
	//				compAnim->controller->current->maxTime = maxFrame / anim->framesPerSecond;
	//			}

	//			compAnim->isPlaying = true;
	//		}
	//	}

	//	ImGui::SameLine();
	//	if (ImGui::Button(">>", ImVec2(23, 23)))
	//	{
	//		if (anim->currentFrame < anim->duration)
	//		{
	//			anim->currentFrame++;
	//			UpdateGameObjectAnimation(App->scene->selected, anim);
	//			compAnim->controller->current->time = anim->currentFrame / anim->framesPerSecond;
	//		}
	//	}

	//	if (isCliping)
	//	{
	//		if (!compAnim->isPlaying)
	//		{
	//			maxFrame = anim->currentFrame;
	//		}

	//		ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::GetWindowWidth() / 6);
	//		ImGui::PushItemWidth(60);
	//		ImGui::DragInt("Frame Start", &minFrame, 1.0f, 0, maxFrame - 1); ImGui::PopItemWidth();
	//		ImGui::SameLine(); ImGui::Text("Frame Start");

	//		ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::GetWindowWidth() / 6);
	//		ImGui::PushItemWidth(60);
	//		ImGui::DragInt("Frame End", &maxFrame, 1.0f, minFrame + 1, anim->duration); ImGui::PopItemWidth();
	//		ImGui::SameLine(); ImGui::Text("Frame End");
	//	}

	//}

	//ImGui::End();
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

void PanelAnimation::CreateAnimationFromClip(ResourceAnimation* anim, int minFrame, int maxFrame)
{
	ResourceAnimation* newAnim = (ResourceAnimation*)App->resManager->CreateNewResource(TYPE::ANIMATION);
	ComponentAnimation* compAnim = ((ComponentAnimation*)(App->scene->selected->GetComponent(ComponentType::Animation)));

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
			newChannel->numPositionKeys = maxFrame - minFrame;
		}

		if (anim->channels[i]->numRotationKeys == 1)
		{
			newChannel->numRotationKeys = 1;
		}
		else
		{
			newChannel->numRotationKeys = maxFrame - minFrame;
		}

		for (int j = 0; j < newChannel->numPositionKeys; ++j)
		{
			math::float3 position = anim->channels[i]->positionSamples[minFrame + j];
			newChannel->positionSamples.push_back(position);
		}

		for (int j = 0; j < newChannel->numRotationKeys; ++j)
		{
			math::Quat rotation = anim->channels[i]->rotationSamples[minFrame + j];
			newChannel->rotationSamples.push_back(rotation);
		}

		newAnim->channels.push_back(newChannel);
	}

	newAnim->duration = maxFrame - minFrame; //frames
	newAnim->framesPerSecond = anim->framesPerSecond;
	newAnim->numberFrames = maxFrame - minFrame;
	newAnim->numberOfChannels = anim->numberOfChannels;
	newAnim->durationInSeconds = (maxFrame - minFrame) / anim->framesPerSecond;
}
