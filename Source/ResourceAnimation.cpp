#include "Resource.h"

#include "Application.h"

#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h"

#include "ResourceAnimation.h"

#include "Globals.h"
#include "imgui.h"
#include "JSON.h"
#include <assert.h>

#include <algorithm>


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

	unsigned ok = App->fsystem->Load(exportedFile.c_str(), &data);

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
	// Check if the animation was imported from Model or created from the editor
	// TODO RM: Delete this if and use base Delete once all animations from models are saved on assets
	if (App->fsystem->Exists(file.c_str()))
	{
		Resource::Delete();
	}
	else
	{
		// Delete Resource from ResourceManager
		App->resManager->DeleteResourceFromList(UID);

		// Delete file in Library
		std::string fileInLibrary(IMPORTED_ANIMATIONS);
		fileInLibrary += exportedFile;
		fileInLibrary += ANIMATIONEXTENSION;
		App->fsystem->Delete(fileInLibrary.c_str());
		DeleteFromMemory();
	}
}

void ResourceAnimation::SaveMetafile(const char* file) const
{
	std::string filepath;
	filepath.append(file);
	JSON* json = new JSON();
	JSON_value* meta = json->CreateValue();
	struct stat statFile;
	stat(filepath.c_str(), &statFile);
	meta->AddUint("metaVersion", META_VERSION);
	meta->AddUint("timeCreated", statFile.st_ctime);

	// Resource info
	meta->AddUint("GUID", UID);
	meta->AddString("Name", name.c_str());
	meta->AddString("File", file);
	meta->AddString("ExportedFile", exportedFile.c_str());

	json->AddValue("Animation", *meta);

	// Save meta in Assets if animation comes from animation file
	if (App->fsystem->GetExtension(filepath) == ANIMATIONEXTENSION)
	{
		filepath += METAEXT;
		App->fsystem->Save(filepath.c_str(), json->ToString().c_str(), json->Size());
	}

	// Save meta in Library
	std::string libraryPath(exportedFile + METAEXT);
	App->fsystem->Save(libraryPath.c_str(), json->ToString().c_str(), json->Size());
	RELEASE(json);
}

void ResourceAnimation::LoadConfigFromMeta()
{
	std::string metaFile(file);
	metaFile += ".meta";

	// Check if meta file exists
	if (!App->fsystem->Exists(metaFile.c_str()))
		return;

	char* data = nullptr;
	unsigned oldUID = GetUID();

	if (App->fsystem->Load(metaFile.c_str(), &data) == 0)
	{
		LOG("Warning: %s couldn't be loaded", metaFile.c_str());
		RELEASE_ARRAY(data);
		return;
	}
	JSON* json = new JSON(data);
	JSON_value* value = json->GetValue("Animation");

	// Make sure the UID from meta is the same
	unsigned checkUID = value->GetUint("GUID");
	if (oldUID != checkUID)
	{
		UID = checkUID;
		// Update resource UID on resource list
		App->resManager->ReplaceResource(oldUID, this);
		exportedFile = IMPORTED_ANIMATIONS + std::to_string(UID) + ANIMATIONEXTENSION;
	}

	// Check the meta file version
	if (value->GetUint("metaVersion", 0u) < META_VERSION)
		SaveMetafile(file.c_str());

	// Check the meta saved in library, if not save it
	if (!App->fsystem->Exists((exportedFile + METAEXT).c_str()))
		SaveMetafile(file.c_str());

	RELEASE_ARRAY(data);
	RELEASE(json);
}

void ResourceAnimation::LoadConfigFromLibraryMeta()
{
	std::string metaFile(exportedFile);
	metaFile += ".meta";

	// Check if meta file exists
	if (!App->fsystem->Exists(metaFile.c_str()))
		return;

	char* data = nullptr;
	unsigned oldUID = GetUID();

	if (App->fsystem->Load(metaFile.c_str(), &data) == 0)
	{
		LOG("Warning: %s couldn't be loaded", metaFile.c_str());
		RELEASE_ARRAY(data);
		return;
	}
	JSON* json = new JSON(data);
	JSON_value* value = json->GetValue("Animation");

	// Get resource variables
	name = value->GetString("Name");
	file = value->GetString("File");

	RELEASE_ARRAY(data);
	RELEASE(json);
}

void ResourceAnimation::SetAnimation(const char* animationData)
{
	const char* data = animationData; //used as a base pointer for release
	for (const auto& channel : channels)
	{
		channel->numPositionKeys = 0u;
		channel->numRotationKeys = 0u;
		channel->positionSamples.clear();
		channel->rotationSamples.clear();
	}
	channels.clear();

	memcpy(&duration, animationData, sizeof(double));
	animationData += sizeof(double);

	memcpy(&framesPerSecond, animationData, sizeof(double));
	animationData += sizeof(double);

	numberFrames = duration;

	durationInSeconds = float(duration * (1 / framesPerSecond));

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

	//load events
	events.clear();

	memcpy(&totalEvents, animationData, sizeof(int));
	animationData += sizeof(int);

	for (unsigned i = 0u; i < totalEvents; i++)
	{
		int newKey;
		memcpy(&newKey, animationData, sizeof(int));
		animationData += sizeof(int);

		int newFrame;
		memcpy(&newFrame, animationData, sizeof(int));
		animationData += sizeof(int);

		char newName[MAX_BONE_NAME_LENGTH];
		memcpy(newName, animationData, sizeof(char) * MAX_BONE_NAME_LENGTH);
		animationData += sizeof(char)* MAX_BONE_NAME_LENGTH;

		Event* newEvent = new Event(newKey, newFrame, std::string(newName));
		events.push_back(newEvent);
	}

	RELEASE_ARRAY(data);
}

unsigned ResourceAnimation::GetAnimationSize()
{
	unsigned size = 0u;

	size += sizeof(double) * 2;
	size += sizeof(int);

	for (unsigned i = 0u; i < numberOfChannels; ++i)
	{
		size += sizeof(char)* MAX_BONE_NAME_LENGTH;
		size += sizeof(int) * 2;

		for (unsigned j = 0u; j < channels[i]->numPositionKeys; ++j)
		{
			size += sizeof(float) * 3;
		}
		for (unsigned j = 0u; j < channels[i]->numRotationKeys; ++j)
		{
			size += sizeof(Quat);
		}
	}

	//get events
	size += sizeof(int);

	for (unsigned i = 0u; i < totalEvents; i++)
	{
		size += sizeof(int) * 2;
		size += sizeof(char)* MAX_BONE_NAME_LENGTH;
	}

	return size;
}

void ResourceAnimation::SaveAnimationData(char * data)
{
	char* cursor = data;

	memcpy(cursor, &duration, sizeof(double));
	cursor += sizeof(double);

	memcpy(cursor, &framesPerSecond, sizeof(double));
	cursor += sizeof(double);

	memcpy(cursor, &numberOfChannels, sizeof(int));
	cursor += sizeof(int);

	for (unsigned j = 0u; j < numberOfChannels; j++)
	{
		memcpy(cursor, channels[j]->channelName.c_str(), sizeof(char) * MAX_BONE_NAME_LENGTH);  //Name
		cursor += sizeof(char) * MAX_BONE_NAME_LENGTH;

		memcpy(cursor, &channels[j]->numPositionKeys, sizeof(int));
		cursor += sizeof(int);

		memcpy(cursor, &channels[j]->numRotationKeys, sizeof(int));
		cursor += sizeof(int);

		//importar longitud array de posiciones e iterar

		for (unsigned i = 0u; i < channels[j]->numPositionKeys; i++)
		{
			memcpy(cursor, &channels[j]->positionSamples[i], sizeof(float) * 3);
			cursor += sizeof(float) * 3;
		}

		//importar longitud array de rotaciones e iterar

		for (unsigned k = 0u; k < channels[j]->numRotationKeys; k++)
		{
			math::Quat newQuat = math::Quat(channels[j]->rotationSamples[k].w, channels[j]->rotationSamples[k].x,
				channels[j]->rotationSamples[k].y, channels[j]->rotationSamples[k].z);

			memcpy(cursor, &newQuat, sizeof(math::Quat));
			cursor += sizeof(math::Quat);
		}
	}

	//save events
	memcpy(cursor, &totalEvents, sizeof(int));
	cursor += sizeof(int);

	for (unsigned i = 0u; i < totalEvents; i++)
	{
		memcpy(cursor, &events[i]->key, sizeof(int));
		cursor += sizeof(int);

		memcpy(cursor, &events[i]->frame, sizeof(int));
		cursor += sizeof(int);

		memcpy(cursor, events[i]->name.c_str(), sizeof(char) * MAX_BONE_NAME_LENGTH);
		cursor += sizeof(char) * MAX_BONE_NAME_LENGTH;
	}

}

void ResourceAnimation::SaveNewAnimation()
{
	char* animationData = nullptr;
	unsigned animationSize = GetAnimationSize();
	animationData = new char[animationSize];
	SaveAnimationData(animationData);

	App->fsystem->Save((ANIMATIONS + name + ANIMATIONEXTENSION).c_str(), animationData, animationSize);
	SetFile((ANIMATIONS + name + ANIMATIONEXTENSION).c_str());
	RELEASE_ARRAY(animationData);
}

void ResourceAnimation::AddEvent(std::string name)
{
	events.push_back(new Event(totalEvents, currentFrame, name));
	++totalEvents;

	if (totalEvents > 1)
	{
		std::sort(events.begin(), events.end(), [](const Event* lhs, const Event* rhs) 
			{ return lhs->frame < rhs->frame; });
	}

	SetEventKeys();
}

void ResourceAnimation::DeleteEvent(int key)
{
	for (std::vector<Event*>::iterator it = events.begin(); it != events.end(); ++it)
	{
		if ((*it)->key == key)
		{
			events.erase(it);
			--totalEvents;
			break;
		}
	}

	SetEventKeys();
}

void ResourceAnimation::SetEventKeys()
{
	int contador = 0;
	for (std::vector<Event*>::iterator it = events.begin(); it != events.end(); ++it)
	{
		(*it)->key = contador;
		++contador;
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
	{
		if (indexPosition >= channels[indexChannel]->positionSamples.size())
		{
			indexPosition = channels[indexChannel]->positionSamples.size() - 1;
		}
		return channels[indexChannel]->positionSamples[indexPosition];
	}
	else
		return channels[indexChannel]->positionSamples[0];
}

const math::Quat ResourceAnimation::GetRotation(unsigned indexChannel, unsigned indexPosition) const
{
	if (channels[indexChannel]->numRotationKeys > 1)
	{
		if (indexPosition >=  channels[indexChannel]->rotationSamples.size())
		{
			indexPosition = channels[indexChannel]->rotationSamples.size() - 1;
		}
		return channels[indexChannel]->rotationSamples[indexPosition];
	}
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
