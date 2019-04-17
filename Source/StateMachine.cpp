#include "StateMachine.h"
#include "Globals.h"


StateMachine::StateMachine()
{
}


StateMachine::~StateMachine()
{
}

void StateMachine::AddClip(const HashString name, Animation* anim, const bool loop)
{
	clips.push_back(Clip(name, anim, loop));
}

void StateMachine::AddNode(const HashString name, const HashString clipName)
{
	nodes.push_back(Node(name, clipName));
}

void StateMachine::AddTransition(const HashString origin, const HashString destiny, const HashString trigger, unsigned blend)
{
	transitions.push_back(Transition(origin, destiny, trigger, blend));
}

unsigned StateMachine::FindClip(HashString name)
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

unsigned StateMachine::FindNode(HashString name)
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

unsigned StateMachine::FindTransition(HashString origin, HashString trigger)
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

HashString StateMachine::GetClipName(unsigned index)
{
	return clips[index].name;
}

Animation* StateMachine::GetClipAnimation(unsigned index)
{
	return clips[index].anim;
}

bool StateMachine::GetClipLoop(unsigned index)
{
	return clips[index].loop;
}

HashString StateMachine::GetTransitionOrigin(unsigned index)
{
	return transitions[index].origin;
}

HashString StateMachine::GetTransitionDestiny(unsigned index)
{
	return transitions[index].destiny;
}

HashString StateMachine::GetTransitionTrigger(unsigned index)
{
	return transitions[index].trigger;
}

unsigned StateMachine::GetTransitionBlend(unsigned index)
{
	return transitions[index].blend;
}

HashString StateMachine::GetNodeName(unsigned index)
{
	return nodes[index].name;
}

HashString StateMachine::GetNodeClip(unsigned index)
{
	return nodes[index].clipName;
}

unsigned StateMachine::GetNodesSize()
{
	return nodes.size();
}

unsigned StateMachine::GetClipsSize()
{
	return clips.size();
}

unsigned StateMachine::GetTransitionsSize()
{
	return transitions.size();
}

void StateMachine::SetClipName(unsigned index, HashString name)
{
	clips[index].name = name;
}

void StateMachine::SetClipAnimation(unsigned index, Animation* anim)
{
	clips[index].anim = anim;
}

void StateMachine::SetClipLoop(unsigned index, bool loop)
{
	clips[index].loop = loop;
}

void StateMachine::SetTransitionOrigin(unsigned index, HashString origin)
{
	transitions[index].origin = origin;
}

void StateMachine::SetTransitionDestiny(unsigned index, HashString destiny)
{
	transitions[index].destiny = destiny;
}

void StateMachine::SetTransitionTrigger(unsigned index, HashString trigger)
{
	transitions[index].trigger = trigger;
}

void StateMachine::SetTransitionBlend(unsigned index, unsigned blend)
{
	transitions[index].blend = blend;
}

void StateMachine::SetNodeName(unsigned index, HashString name)
{
	nodes[index].name = name;
}

void StateMachine::SetNodeClip(unsigned index, HashString clipName)
{
	nodes[index].clipName = clipName;
}

void StateMachine::RemoveClip(unsigned UID)
{

}

void StateMachine::RemoveNode(unsigned index)
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

void StateMachine::RemoveTransition(unsigned index)
{
	transitions.erase(transitions.begin() + index);
}

void StateMachine::RemoveNodeTransitions(HashString nodeName)
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