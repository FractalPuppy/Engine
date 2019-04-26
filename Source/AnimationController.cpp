#include "Application.h"


#include "AnimationController.h"

#include "ModuleResourceManager.h"

#include "ResourceAnimation.h"
#include "Brofiler.h"

AnimationController::AnimationController()
{
}


AnimationController::~AnimationController()
{
}

void AnimationController::Play(ResourceAnimation* anim, bool loop, bool mustFinish, float speed, int startFrame, int endFrame)
{
	Instance* newInstance = new Instance;
	newInstance->anim = anim;
	newInstance->mustFinish = mustFinish;
	newInstance->speed = speed;
	newInstance->time = startFrame / (current->anim->framesPerSecond);
	newInstance->minTime = startFrame / (current->anim->framesPerSecond);
	newInstance->maxTime = endFrame / (current->anim->framesPerSecond);
	newInstance->loop = loop;
	current = newInstance;
}

void AnimationController::PlayEditor(ResourceAnimation * anim)
{
	Instance* newInstance = new Instance;
	newInstance->anim = anim;
	newInstance->loop = true;
	current = newInstance;
}

void AnimationController::PlayNextNode(ResourceAnimation * anim, bool loop, bool mustFinish, float speed, int startFrame, int endFrame, unsigned blend)
{
	current->anim = anim;
	current->loop = loop;
	current->mustFinish = mustFinish;
	current->speed = speed;
	current->time = startFrame / (current->anim->framesPerSecond);
	current->minTime = startFrame / (current->anim->framesPerSecond);
	current->maxTime = endFrame / (current->anim->framesPerSecond);
}



void AnimationController::Update(float dt)
{
	PROFILE;
	if (current != nullptr && !current->isEditor)
	{
		UpdateInstance(current, dt);
	}
	else if (current != nullptr && current->isEditor)
	{
		UpdateEditorInstance(current, dt);
	}
}

void AnimationController::UpdateInstance(Instance* instance, float dt)
{
	ResourceAnimation* anim = instance->anim;

	if (anim != nullptr && anim->durationInSeconds > 0)
	{
		float trueDt = dt * instance->speed;

		if (trueDt > 0.0f)
		{
			float timeRemainingA = current->maxTime - instance->time;
			LOG( std::to_string(timeRemainingA).c_str());
			if (trueDt <= timeRemainingA)
			{
				instance->time += trueDt;
				trueFrame = instance->time * anim->framesPerSecond;
				anim->currentFrame = (int)trueFrame;
			}
			else if (instance->loop)
			{
				instance->time = current->minTime + trueDt - timeRemainingA;
				trueFrame = current->maxTime;
			}
			else
			{
				instance->time = current->maxTime;
			}
		}
		else
		{
			float timeRemainingA = current->maxTime - current->minTime - instance->time;
			if (trueDt >= timeRemainingA)
			{
				instance->time += trueDt;
				trueFrame = instance->time * anim->framesPerSecond;
				anim->currentFrame = (int)trueFrame;
			}
			else if (instance->loop)
			{
				instance->time = instance->maxTime - timeRemainingA + trueDt;
				trueFrame = anim->duration;
			}
			else
			{
				instance->time = instance->minTime;
			}
		}
	}

	//We'll have two lists of events one that will be emptying itself checking for scripts audio etc

	if (instance->next != nullptr)
	{
		float timeRemainingB = instance->fadeDuration - instance->fadeTime;
		if (dt <= timeRemainingB)
		{
			instance->fadeTime += dt;
			UpdateInstance(instance->next, dt);
		}
		else
		{
			ReleaseInstance(instance->next);
			instance->next = nullptr;
			instance->fadeTime = instance->fadeDuration = 0;
		}
	}
}

void AnimationController::UpdateEditorInstance(Instance* instance, float dt)
{
	ResourceAnimation* anim = instance->anim;

	if (anim != nullptr && anim->durationInSeconds > 0)
	{
		float trueDt = dt * instance->speed;

		if (trueDt > 0.0f)
		{
			float timeRemainingA = current->maxTime - instance->time;
			if (trueDt <= timeRemainingA)
			{
				instance->time += trueDt;
				trueFrame = instance->time * anim->framesPerSecond;
				anim->currentFrame = (int)trueFrame;
			}
			else if (instance->loop)
			{
				instance->time = current->minTime + trueDt - timeRemainingA;
				trueFrame = current->maxTime;
			}
			else
			{
				instance->time = current->maxTime;
			}
		}
		else
		{
			float timeRemainingA = current->maxTime - current->minTime - instance->time;
			if (trueDt >= timeRemainingA)
			{
				instance->time += trueDt;
			}
			else if (instance->loop)
			{
				instance->time = instance->maxTime - timeRemainingA + trueDt;
			}
			else
			{
				instance->time = instance->minTime;
			}
		}
	}

	//We'll have two lists of events one that will be emptying itself checking for scripts audio etc

	if (instance->next != nullptr)
	{
		float timeRemainingB = instance->fadeDuration - instance->fadeTime;
		if (dt <= timeRemainingB)
		{
			instance->fadeTime += dt;
			UpdateInstance(instance->next, dt);
		}
		else
		{
			ReleaseInstance(instance->next);
			instance->next = nullptr;
			instance->fadeTime = instance->fadeDuration = 0;
		}
	}
}

void AnimationController::ReleaseInstance(Instance* instance)
{
	do
	{
		Instance* next = instance->next;
		delete instance;
		instance = next;
	} while (instance != nullptr);
}

bool AnimationController::CanSwitch()
{
	bool ret;

	if (current->mustFinish)
	{
		if (current->time == current->anim->durationInSeconds)
		{
			ret = true;
		}
		else
		{
			ret = false;
		}
	}
	else
	{
		ret = true;
	}

	return ret;
}

void AnimationController::ResetClipping()
{
	current->minTime = 0.0f;
	current->maxTime = current->anim->durationInSeconds;
}

bool AnimationController::GetTransform(unsigned channelIndex, math::float3& position, math::Quat& rotation)
{
	if (current != nullptr)
	{
		return GetTransformInstance(current, channelIndex, position, rotation);
	}
	else
		return false;
}

bool AnimationController::GetTransformInstance(Instance* instance, unsigned channelIndex, math::float3& position, math::Quat& rotation)
{
	ResourceAnimation* anim = instance->anim;

	if (anim != nullptr)
	{

		//Used to know how far are we from each frame

		float positionKey = float(instance->time*(anim->GetNumPositions(channelIndex) - 1)) / float(anim->durationInSeconds);
		float rotationKey = float(instance->time*(anim->GetNumRotations(channelIndex) - 1)) / float(anim->durationInSeconds);

		unsigned positionIndex = unsigned(positionKey);
		unsigned rotationIndex = unsigned(rotationKey);
			
		float positionLambda = positionKey - float(positionIndex);
		float rotationLambda = rotationKey - float(rotationIndex);

		if (positionLambda > 0.0f)
		{
			position = InterpolateFloat3(anim->GetPosition(channelIndex, positionIndex), anim->GetPosition(channelIndex, positionIndex + 1), positionLambda);
		}
		else
		{
			position = anim->GetPosition(channelIndex, positionIndex);
		}
		if (rotationLambda > 0.0f)
		{
			rotation = InterpolateQuat(anim->GetRotation(channelIndex, rotationIndex), anim->GetRotation(channelIndex, rotationIndex + 1), rotationLambda);
		}
		else
		{
			rotation = anim->GetRotation(channelIndex, rotationIndex);
		}
		if (instance->next != nullptr)
		{
			assert(instance->fadeDuration > 0.0f);

			math::float3 nextPosition = math::float3::zero;
			math::Quat nextRotation = math::Quat::identity;

			if (GetTransformInstance(instance->next, channelIndex, nextPosition, nextRotation))
			{
				float blendLambda = float(instance->fadeTime) / float(instance->fadeDuration);

				position = InterpolateFloat3(nextPosition, position, blendLambda);
				rotation = InterpolateQuat(nextRotation, rotation, blendLambda);
			}
		}
		return true;

	}
	return false;
}

math::float3 AnimationController::InterpolateFloat3(const math::float3& first, const math::float3& second, float lambda) const
{
	return first * (1.0f - lambda) + second * lambda;
}

math::Quat AnimationController::InterpolateQuat(const math::Quat& first, const math::Quat& second, float lambda) const
{
	Quat result;
	float dot = first.Dot(second);

	if (dot >= 0.0f) // Interpolate through the shortest path
	{
		result.x = first.x*(1.0f - lambda) + second.x*lambda;
		result.y = first.y*(1.0f - lambda) + second.y*lambda;
		result.z = first.z*(1.0f - lambda) + second.z*lambda;
		result.w = first.w*(1.0f - lambda) + second.w*lambda;
	}
	else
	{
		result.x = first.x*(1.0f - lambda) - second.x*lambda;
		result.y = first.y*(1.0f - lambda) - second.y*lambda;
		result.z = first.z*(1.0f - lambda) - second.z*lambda;
		result.w = first.w*(1.0f - lambda) - second.w*lambda;
	}

	result.Normalize();

	return result;
}