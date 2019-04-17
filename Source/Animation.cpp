#include "Application.h"

#include "Animation.h"
#include "Globals.h"

#include <assert.h>


Animation::Animation()
{
}


Animation::~Animation()
{
	for (const auto& channel : channels)
	{
		delete channel;
	}
}

void Animation::Load(const char* animationData, unsigned uid)
{

	UID = uid;

	memcpy(&duration, animationData, sizeof(double));
	animationData += sizeof(double);

	memcpy(&framesPerSecond, animationData, sizeof(double));
	animationData += sizeof(double);

	numberFrames = duration;

	durationInSeconds = duration * (1/framesPerSecond);

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

unsigned Animation::GetNumPositions(unsigned indexChannel) const
{
	return channels[indexChannel]->numPositionKeys;
}

unsigned Animation::GetNumRotations(unsigned indexChannel) const
{
	return channels[indexChannel]->numRotationKeys;
}

const math::float3 Animation::GetPosition(unsigned indexChannel, unsigned indexPosition) const
{
	if(channels[indexChannel]->numPositionKeys > 1)
		return channels[indexChannel]->positionSamples[indexPosition];
	else
		return channels[indexChannel]->positionSamples[0];
}

const math::Quat Animation::GetRotation(unsigned indexChannel, unsigned indexPosition) const
{
	if (channels[indexChannel]->numRotationKeys > 1)
		return channels[indexChannel]->rotationSamples[indexPosition];
	else
		return channels[indexChannel]->rotationSamples[0];
}

unsigned Animation::GetIndexChannel(std::string name) const
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