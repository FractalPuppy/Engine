#ifndef __ANIMATION_H_
#define __ANIMATION_H_

#include <vector>
#include "Math/float3.h"
#include "Math/float4x4.h"
#include "Math/Quat.h"


struct Channel
{
	std::string channelName = "";
	unsigned numPositionKeys = 0u;
	unsigned numRotationKeys = 0u;
	std::vector<math::float3> positionSamples;
	std::vector<math::Quat> rotationSamples;
};

class Animation
{
public:
	Animation();
	~Animation();
public:
	void Load(const char* animationData, unsigned uid);
	void Unload();

	unsigned GetNumPositions(unsigned indexChannel) const;
	const math::float3 GetPosition(unsigned indexChannel, unsigned indexPosition) const;
	
	unsigned GetNumRotations(unsigned indexChannel) const;
	const math::Quat GetRotation(unsigned indexChannel, unsigned indexPosition) const;

	unsigned GetIndexChannel(std::string name) const;

public:

	std::string animationName;
	double duration = 0; //frames
	double framesPerSecond = 0;
	int numberFrames = 0;
	unsigned numberOfChannels = 0u;
	unsigned durationInSeconds = 0u;

	unsigned UID = 0u;

	int currentSample = 0u;

	std::vector<Channel*> channels;
};

#endif // __ANIMATION_H_