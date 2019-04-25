#include "Resource.h"

#include "Application.h"

#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h"

#include "ResourceAnimation.h"
#include "Globals.h"

#include <assert.h>


ResourceAnimation::ResourceAnimation(unsigned uid) : Resource(uid, TYPE::ANIMATION)
{
}

ResourceAnimation::ResourceAnimation(const ResourceAnimation& resource) : Resource(resource)
{
	for (int i = 0; i < resource.numberOfChannels; ++i)
	{
		Channel* newChannel = new Channel();
		
		newChannel->channelName = resource.channels[i]->channelName;
		newChannel->numPositionKeys = resource.channels[i]->numPositionKeys;
		newChannel->numRotationKeys = resource.channels[i]->numRotationKeys;
		
		for (int j = 0; j < newChannel->numPositionKeys; ++j)
		{
			newChannel->positionSamples[j] = resource.channels[i]->positionSamples[j];
		}

		for (int j = 0; j < newChannel->numRotationKeys; ++j)
		{
			newChannel->rotationSamples[j] = resource.channels[i]->rotationSamples[j];
		}

		channels.push_back(newChannel);
	}

	duration = resource.duration; //frames
	framesPerSecond = resource.framesPerSecond;
	numberFrames = resource.numberFrames;
	numberOfChannels = resource.numberOfChannels;
	durationInSeconds = resource.durationInSeconds;
}


ResourceAnimation::~ResourceAnimation()
{
	DeleteFromMemory();
}

bool ResourceAnimation::LoadInMemory()
{
	char* data = nullptr;

	unsigned ok = App->fsystem->Load((ANIMATIONS + std::to_string(UID) + ANIMATIONEXTENSION).c_str(), &data);

	// Load mesh file
	if (ok != 0)
	{
		SetAnimation(data);
		++loaded;
	}
	return true;
}

void ResourceAnimation::DeleteFromMemory()
{
	Resource::DeleteFromMemory();
	for(const auto& channel : channels)
	{
		channel->numPositionKeys = 0u;
		channel->numRotationKeys = 0u;
		channel->positionSamples.clear();
		channel->rotationSamples.clear();
	}
	channels.clear();
}

void ResourceAnimation::Delete()
{
	// Delete Resource from ResourceManager
	App->resManager->DeleteResourceFromList(UID);

	// Delete file in Library
	std::string fileInLibrary(ANIMATIONS);
	fileInLibrary += exportedFileName;
	fileInLibrary += ANIMATIONEXTENSION;
	App->fsystem->Delete(fileInLibrary.c_str());
	DeleteFromMemory();
}

void ResourceAnimation::SetAnimation(const char* animationData)
{
	memcpy(&duration, animationData, sizeof(double));
	animationData += sizeof(double);

	memcpy(&framesPerSecond, animationData, sizeof(double));
	animationData += sizeof(double);

	numberFrames = duration;

	durationInSeconds = duration * (1 / framesPerSecond);

	memcpy(&numberOfChannels, animationData, sizeof(int));
	animationData += sizeof(int);

	for (unsigned i = 0u; i < numberOfChannels; i++)
	{
		Channel* newChannel = new Channel();

		char name[MAX_BONE_NAME_LENGTH]; //crash fix - Here we don't need to care of namelength. The /0 marks the end of the string. Notice the strings are length + 1 ALWAYS

		memcpy(name, animationData, sizeof(char) * MAX_BONE_NAME_LENGTH);
		animationData += sizeof(char)* MAX_BONE_NAME_LENGTH;

		newChannel->channelName = std::string(name);

		memcpy(&newChannel->numPositionKeys, animationData, sizeof(int));
		animationData += sizeof(int);

		memcpy(&newChannel->numRotationKeys, animationData, sizeof(int));
		animationData += sizeof(int);

		for (unsigned j = 0u; j < newChannel->numPositionKeys; j++)
		{
			math::float3 position = math::float3::one;
			memcpy(&position, animationData, sizeof(float) * 3);
			animationData += sizeof(float) * 3;
			newChannel->positionSamples.push_back(position);
		}

		for (unsigned k = 0u; k < newChannel->numRotationKeys; k++)
		{
			math::Quat assimpRotation = math::Quat::identity;
			memcpy(&assimpRotation, animationData, sizeof(math::Quat));
			animationData += sizeof(math::Quat);

			math::Quat rotation = math::Quat(assimpRotation.y, assimpRotation.z, assimpRotation.w, assimpRotation.x);
			newChannel->rotationSamples.push_back(rotation);
		}

		channels.push_back(newChannel);
	}


}

unsigned ResourceAnimation::GetNumPositions(unsigned indexChannel) const
{
	return channels[indexChannel]->numPositionKeys;
}

unsigned ResourceAnimation::GetNumRotations(unsigned indexChannel) const
{
	return channels[indexChannel]->numRotationKeys;
}

const math::float3 ResourceAnimation::GetPosition(unsigned indexChannel, unsigned indexPosition) const
{
	if (channels[indexChannel]->numPositionKeys > 1)
		return channels[indexChannel]->positionSamples[indexPosition];
	else
		return channels[indexChannel]->positionSamples[0];
}

const math::Quat ResourceAnimation::GetRotation(unsigned indexChannel, unsigned indexPosition) const
{
	if (channels[indexChannel]->numRotationKeys > 1)
		return channels[indexChannel]->rotationSamples[indexPosition];
	else
		return channels[indexChannel]->rotationSamples[0];
}

unsigned ResourceAnimation::GetIndexChannel(std::string name) const
{
	for (unsigned i = 0u; i < numberOfChannels; i++) //TODO, guardar map, en el anim controller
	{
		if (strcmp(name.c_str(), channels[i]->channelName.c_str()) == 0)
		{
			return i;
		}
	}
	return 999u;
}

unsigned ResourceAnimation::GetNumberChannels() const
{
	return channels.size();
}

unsigned ResourceAnimation::GetNumberFrames() const
{
	return unsigned(numberFrames);
}

unsigned ResourceAnimation::GetFPS() const
{
	return unsigned(framesPerSecond);
}
