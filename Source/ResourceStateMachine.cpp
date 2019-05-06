#include "Application.h"

#include "ModuleFileSystem.h"

#include "ModuleResourceManager.h"
#include "ResourceStateMachine.h"
#include "ResourceAnimation.h"

#include "Globals.h"


ResourceStateMachine::ResourceStateMachine(unsigned uid) : Resource(uid, TYPE::STATEMACHINE)
{
}

ResourceStateMachine::ResourceStateMachine(const ResourceStateMachine& resource) : Resource(resource)
{
	name = resource.name;
	//We need to copy all the transitions, nodes and clips

	for(const auto& clip : resource.clips)
	{
		clips.push_back(Clip(clip.name, clip.UID, clip.loop, clip.mustFinish, clip.clipSpeed, clip.startFrame , clip.endFrame));
	}

	for (const auto& node : resource.nodes)
	{
		nodes.push_back(Node(node.name, node.clipName));
	}

	for (const auto& transition : resource.transitions)
	{
		transitions.push_back(Transition(transition.origin, transition.destiny, transition.trigger, transition.blend));
	}
}

ResourceStateMachine::~ResourceStateMachine()
{
	DeleteFromMemory();
}

bool ResourceStateMachine::LoadInMemory()
{
	char* data = nullptr;

	unsigned ok = App->fsystem->Load((STATEMACHINES + std::to_string(GetUID()) + STATEMACHINEEXTENSION).c_str(), &data);

	// Load mesh file
	if (ok != 0)
	{
		SetStateMachine(data); //Deallocates data
		++loaded;
	}
	return true;
}

void ResourceStateMachine::DeleteFromMemory()
{
	Resource::DeleteFromMemory();
	defaultNode = 0u;
	nodes.clear();
	clips.clear();
	transitions.clear();

	App->fsystem->Remove((STATEMACHINES + std::to_string(GetUID()) + STATEMACHINEEXTENSION).c_str());
}

void ResourceStateMachine::Delete()
{
	// Delete Resource from ResourceManager
	App->resManager->DeleteResourceFromList(UID);

	// Delete file in Library
	std::string fileInLibrary(STATEMACHINES);
	fileInLibrary += exportedFileName;
	fileInLibrary += STATEMACHINEEXTENSION;
	App->fsystem->Delete(fileInLibrary.c_str());
	DeleteFromMemory();
}

void ResourceStateMachine::SetStateMachine(const char* data)
{
	defaultNode = 0u;
	nodes.clear();
	clips.clear();
	transitions.clear();

	char smName[MAX_BONE_NAME_LENGTH];

	memcpy(smName, data, sizeof(char) * MAX_BONE_NAME_LENGTH);
	data += sizeof(char)* MAX_BONE_NAME_LENGTH;

	name = std::string(smName);

	//import clips vector
	unsigned clipsSize = 0u;
	memcpy(&clipsSize, data, sizeof(unsigned));
	data += sizeof(int);

	for (unsigned i = 0u; i < clipsSize; ++i)
	{
		char CN[MAX_BONE_NAME_LENGTH];

		memcpy(CN, data, sizeof(char) * MAX_BONE_NAME_LENGTH);
		data += sizeof(char)* MAX_BONE_NAME_LENGTH;

		HashString clipName(CN);

		unsigned uid = 0u;

		memcpy(&uid, data, sizeof(int));
		data += sizeof(int);

		bool loop;
		memcpy(&loop, data, sizeof(bool));
		data += sizeof(bool);

		bool finish;
		memcpy(&finish, data, sizeof(bool));
		data += sizeof(bool);

		float speed;
		memcpy(&speed, data, sizeof(float));
		data += sizeof(float);

		int startFrame;
		memcpy(&startFrame, data, sizeof(int));
		data += sizeof(int);
		
		int endFrame;
		memcpy(&endFrame, data, sizeof(int));
		data += sizeof(int);

		clips.push_back(Clip(clipName, uid, loop, finish, speed, startFrame, endFrame));
	}

	//import nodes vector
	unsigned nodesSize = 0u;
	memcpy(&nodesSize, data, sizeof(unsigned));
	data += sizeof(int);

	for (unsigned i = 0u; i < nodesSize; ++i)
	{
		char nodeName[MAX_BONE_NAME_LENGTH];

		memcpy(nodeName, data, sizeof(char) * MAX_BONE_NAME_LENGTH);
		data += sizeof(char)* MAX_BONE_NAME_LENGTH;

		HashString nName(nodeName);

		char nodeClipName[MAX_BONE_NAME_LENGTH];

		memcpy(nodeClipName, data, sizeof(char) * MAX_BONE_NAME_LENGTH);
		data += sizeof(char)* MAX_BONE_NAME_LENGTH;

		HashString ncName(nodeClipName);

		nodes.push_back(Node(nName, ncName));
	}


	//import transitions vector
	unsigned transitionsSize = 0u;
	memcpy(&transitionsSize, data, sizeof(unsigned));
	data += sizeof(int);

	for (unsigned i = 0u; i < transitionsSize; ++i)
	{
		//origin destiny trigger blend
		char tOrigin[MAX_BONE_NAME_LENGTH];
		memcpy(tOrigin, data, sizeof(char) * MAX_BONE_NAME_LENGTH);
		data += sizeof(char)* MAX_BONE_NAME_LENGTH;
		HashString transitionOrigin(tOrigin);

		char tDestiny[MAX_BONE_NAME_LENGTH];
		memcpy(tDestiny, data, sizeof(char) * MAX_BONE_NAME_LENGTH);
		data += sizeof(char)* MAX_BONE_NAME_LENGTH;
		HashString transitionDestiny(tDestiny);

		char tTrigger[MAX_BONE_NAME_LENGTH];
		memcpy(tTrigger, data, sizeof(char) * MAX_BONE_NAME_LENGTH);
		data += sizeof(char)* MAX_BONE_NAME_LENGTH;
		HashString transitionTrigger(tTrigger);

		unsigned blend = 0u;
		memcpy(&blend, data, sizeof(int));
		data += sizeof(int);

		transitions.push_back(Transition(transitionOrigin, transitionDestiny, transitionTrigger, blend));
	}

	memcpy(&defaultNode, data, sizeof(int));
	data += sizeof(int);
}

unsigned ResourceStateMachine::GetStateMachineSize()
{
	unsigned size = 0u;
	size += sizeof(char) * MAX_BONE_NAME_LENGTH;

	size += sizeof(int);

	for (const auto& clip : clips)
	{
		size += sizeof(char) * MAX_BONE_NAME_LENGTH;
		size += sizeof(int);
		size += sizeof(bool);
		size += sizeof(bool);
		size += sizeof(float);
		size += sizeof(int) * 2;
	}

	size += sizeof(int);
	for (const auto& node : nodes)
	{
		size += sizeof(char) * MAX_BONE_NAME_LENGTH;
		size += sizeof(char) * MAX_BONE_NAME_LENGTH;
	}

	size += sizeof(int);

	for (const auto& transition : transitions)
	{
		size += sizeof(char) * MAX_BONE_NAME_LENGTH;
		size += sizeof(char) * MAX_BONE_NAME_LENGTH;
		size += sizeof(char) * MAX_BONE_NAME_LENGTH;
		size += sizeof(int);
	}
	size += sizeof(int);

	return size;
}

void ResourceStateMachine::SaveStateMachineData(char* data)
{
	char* cursor = data;

	memcpy(cursor, name.c_str(), sizeof(char) * MAX_BONE_NAME_LENGTH);
	cursor += sizeof(char) * MAX_BONE_NAME_LENGTH;

	unsigned clipsSize = clips.size();
	memcpy(cursor, &clipsSize, sizeof(int));
	cursor += sizeof(int);

	for (const auto& clip : clips)
	{
		memcpy(cursor, clip.name.C_str(), sizeof(char) * MAX_BONE_NAME_LENGTH); 
		cursor += sizeof(char) * MAX_BONE_NAME_LENGTH;

		memcpy(cursor, &clip.UID, sizeof(int));
		cursor += sizeof(int);

		memcpy(cursor, &clip.loop, sizeof(bool));
		cursor += sizeof(bool);

		memcpy(cursor, &clip.mustFinish, sizeof(bool));
		cursor += sizeof(bool);

		memcpy(cursor, &clip.clipSpeed, sizeof(float));
		cursor += sizeof(float);

		memcpy(cursor, &clip.startFrame, sizeof(int));
		cursor += sizeof(int);

		memcpy(cursor, &clip.endFrame, sizeof(int));
		cursor += sizeof(int);
	}

	unsigned nodeSize = nodes.size();
	memcpy(cursor, &nodeSize, sizeof(int));
	cursor += sizeof(int);

	for (const auto& node : nodes)
	{
		memcpy(cursor, node.name.C_str(), sizeof(char) * MAX_BONE_NAME_LENGTH); 
		cursor += sizeof(char) * MAX_BONE_NAME_LENGTH;

		memcpy(cursor, node.clipName.C_str(), sizeof(char) * MAX_BONE_NAME_LENGTH); 
		cursor += sizeof(char) * MAX_BONE_NAME_LENGTH;
	}

	unsigned transitionSize = transitions.size();
	memcpy(cursor, &transitionSize, sizeof(int));
	cursor += sizeof(int);
	
	for (const auto& transition : transitions)
	{
		memcpy(cursor, transition.origin.C_str(), sizeof(char) * MAX_BONE_NAME_LENGTH);  
		cursor += sizeof(char) * MAX_BONE_NAME_LENGTH;

		memcpy(cursor, transition.destiny.C_str(), sizeof(char) * MAX_BONE_NAME_LENGTH); 
		cursor += sizeof(char) * MAX_BONE_NAME_LENGTH;

		memcpy(cursor, transition.trigger.C_str(), sizeof(char) * MAX_BONE_NAME_LENGTH); 
		cursor += sizeof(char) * MAX_BONE_NAME_LENGTH;

		memcpy(cursor, &transition.blend, sizeof(int));
		cursor += sizeof(int);
	}

	memcpy(cursor, &defaultNode, sizeof(int));
	cursor += sizeof(int);
}

void ResourceStateMachine::Save()
{

	char* stateMachineData = nullptr;
	unsigned stateMachineSize = GetStateMachineSize();
	stateMachineData = new char[stateMachineSize];
	SaveStateMachineData(stateMachineData);

	App->fsystem->Save((STATEMACHINES + std::to_string(GetUID()) + STATEMACHINEEXTENSION).c_str(), stateMachineData, stateMachineSize);
	SetFile(STATEMACHINES);
	SetExportedFile(std::to_string(GetUID()).c_str());
	RELEASE_ARRAY(stateMachineData);
}

void ResourceStateMachine::AddClip(const HashString name, unsigned UID, const bool loop)
{
	clips.push_back(Clip(name, UID, loop, false, 1.0f ,0 ,0));
}

void ResourceStateMachine::AddNode(const HashString name, const HashString clipName)
{
	nodes.push_back(Node(name, clipName));
}

void ResourceStateMachine::AddTransition(const HashString origin, const HashString destiny, const HashString trigger, unsigned blend)
{
	transitions.push_back(Transition(origin, destiny, trigger, blend));
}

unsigned ResourceStateMachine::FindClip(const HashString name)
{
	unsigned i;

	for (i = 0u; i < clips.size(); ++i)
	{
		if (clips[i].name == name)
		{
			break;
		}
	}
	return i;
}

unsigned ResourceStateMachine::FindNode(HashString name)
{
	unsigned i;

	for (i = 0u; i < nodes.size(); ++i)
	{
		if (nodes[i].name == name)
		{
			break;
		}
	}
	return i;
}

unsigned ResourceStateMachine::FindTransition(HashString origin, HashString trigger)
{
	unsigned i;

	for (i = 0u; i < transitions.size(); ++i)
	{
		if (transitions[i].origin == origin && transitions[i].trigger == trigger)
		{
			break;
		}
	}
	return i;
}

HashString ResourceStateMachine::GetClipName(unsigned index)
{
	return clips[index].name;
}

unsigned ResourceStateMachine::GetClipResource(unsigned index)
{
	return clips[index].UID;
}

bool ResourceStateMachine::GetClipLoop(unsigned index)
{
	return clips[index].loop;
}

int ResourceStateMachine::GetClipStartFrame(unsigned index)
{
	return clips[index].startFrame;
}

int ResourceStateMachine::GetClipEndFrame(unsigned index)
{
	return clips[index].endFrame;
}

bool ResourceStateMachine::GetClipMustFinish(unsigned index)
{
	return clips[index].mustFinish;
}

float ResourceStateMachine::GetClipSpeed(unsigned index)
{
	return clips[index].clipSpeed;
}

HashString ResourceStateMachine::GetTransitionOrigin(unsigned index)
{
	return transitions[index].origin;
}

HashString ResourceStateMachine::GetTransitionDestiny(unsigned index)
{
	return transitions[index].destiny;
}

HashString ResourceStateMachine::GetTransitionTrigger(unsigned index)
{
	return transitions[index].trigger;
}

unsigned ResourceStateMachine::GetTransitionBlend(unsigned index)
{
	return transitions[index].blend;
}

HashString ResourceStateMachine::GetNodeName(unsigned index)
{
	return nodes[index].name;
}

HashString ResourceStateMachine::GetNodeClip(unsigned index)
{
	return nodes[index].clipName;
}

unsigned ResourceStateMachine::GetNodesSize()
{
	return nodes.size();
}

unsigned ResourceStateMachine::GetClipsSize()
{
	return clips.size();
}

unsigned ResourceStateMachine::GetTransitionsSize()
{
	return transitions.size();
}

void ResourceStateMachine::SetClipName(unsigned index, HashString name)
{
	clips[index].name = name;
}

void ResourceStateMachine::SetClipResource(unsigned index, unsigned UID)
{
	clips[index].UID = UID;
}

void ResourceStateMachine::SetClipLoop(unsigned index, bool loop)
{
	clips[index].loop = loop;
}

void ResourceStateMachine::SetClipStartFrame(unsigned index, int startTime)
{
	clips[index].startFrame = startTime;
}

void ResourceStateMachine::SetClipEndFrame(unsigned index, int endTime)
{
	clips[index].endFrame = endTime;
}

void ResourceStateMachine::SetClipSpeed(unsigned index, float speed)
{
	clips[index].clipSpeed = speed;
}

void ResourceStateMachine::SetClipMustFinish(unsigned index, bool mustFinish)
{
	clips[index].mustFinish = mustFinish;
}

void ResourceStateMachine::SetTransitionOrigin(unsigned index, HashString origin)
{
	transitions[index].origin = origin;
}

void ResourceStateMachine::SetTransitionDestiny(unsigned index, HashString destiny)
{
	transitions[index].destiny = destiny;
}

void ResourceStateMachine::SetTransitionTrigger(unsigned index, HashString trigger)
{
	transitions[index].trigger = trigger;
}

void ResourceStateMachine::SetTransitionBlend(unsigned index, unsigned blend)
{
	transitions[index].blend = blend;
}

void ResourceStateMachine::RenameTransitionDueNodeChanged(HashString previous, HashString newName)
{
	for (auto& trans : transitions)
	{
		if (trans.origin == previous)
			trans.origin = newName;

		if (trans.destiny == previous)
			trans.destiny = newName;
	}
}

void ResourceStateMachine::SetNodeName(unsigned index, HashString name)
{
	nodes[index].name = name;
}

void ResourceStateMachine::SetNodeClip(unsigned index, HashString clipName)
{
	nodes[index].clipName = clipName;
}

void ResourceStateMachine::RemoveClip(unsigned uid)
{
	std::vector<Node>::iterator it = nodes.begin();

	while (it != nodes.end())
	{
		if (it->clipName == clips[uid].name)
		{
			RemoveNodeTransitions(it->name);
			it = nodes.erase(it);
		}
		else
		{
			++it;
		}
	}

	clips.erase(clips.begin() + uid);
}

void ResourceStateMachine::RemoveNode(unsigned index)
{
	RemoveNodeTransitions(nodes[index].name);
	nodes.erase(nodes.begin() + index);

	if (nodes.empty())
	{
		defaultNode = 0u;
	}
	else
	{
		if (defaultNode > nodes.size() - 1)
		{
			defaultNode = nodes.size()-1;
		}

	}
}

void ResourceStateMachine::RemoveTransition(unsigned index)
{
	transitions.erase(transitions.begin() + index);
}

void ResourceStateMachine::ReceiveTrigger(HashString trigger, unsigned &blend)
{
	HashString defaultNodeName = GetNodeName(defaultNode);

	for (auto& trans : transitions)
	{
		if (trans.origin == defaultNodeName && trans.trigger == trigger)
		{
			defaultNode = FindNode(trans.destiny);
			blend = trans.blend;
			break;
		}
	}
}

void ResourceStateMachine::RemoveNodeTransitions(HashString nodeName)
{
	std::vector<Transition>::iterator it = transitions.begin();

	while (it != transitions.end())
	{
		if (it->origin == nodeName || it->destiny == nodeName)
		{
			it = transitions.erase(it);
		}
		else
		{
			++it;
		}
	}
}