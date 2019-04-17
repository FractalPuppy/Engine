#ifndef __STATEMACHINE_H_
#define __STATEMACHINE_H_


#include "HashString.h"
#include <vector>

class Animation;

class StateMachine
{
public:


	StateMachine();
	~StateMachine();

public:
	
	void AddClip(const HashString name, Animation* anim, bool loop);
	void AddNode(const HashString name,const HashString clipName);
	void AddTransition(const HashString origin, const HashString destiny, const HashString trigger, unsigned blend);

	unsigned FindClip(const HashString name);
	unsigned FindNode(const HashString name);
	unsigned FindTransition(const HashString origin, const HashString trigger);

	//Clips setters and getters

	HashString GetClipName(unsigned index);
	Animation* GetClipAnimation(unsigned index);
	bool GetClipLoop(unsigned index);
	void SetClipName(unsigned index, HashString name);
	void SetClipAnimation(unsigned index, Animation* anim);
	void SetClipLoop(unsigned index, bool loop);

	//Transitions setters and getters
	HashString GetTransitionOrigin(unsigned index);
	HashString GetTransitionDestiny(unsigned index);
	HashString GetTransitionTrigger(unsigned index);
	unsigned GetTransitionBlend(unsigned index);
	void SetTransitionOrigin(unsigned index, HashString origin);
	void SetTransitionDestiny(unsigned index, HashString destiny);
	void SetTransitionTrigger(unsigned index, HashString trigger);
	void SetTransitionBlend(unsigned index, unsigned blend);

	//Nodes setters and getters
	HashString GetNodeName(unsigned index);
	HashString GetNodeClip(unsigned index);
	void SetNodeName(unsigned index, HashString name);
	void SetNodeClip(unsigned index, HashString clipName);

	unsigned GetNodesSize();
	unsigned GetClipsSize();
	unsigned GetTransitionsSize();

	void RemoveNodeTransitions(HashString transitionName);

	void RemoveClip(unsigned UID);
	void RemoveNode(unsigned UID);
	void RemoveTransition(unsigned UID);

private:
	
	struct Clip
	{
		HashString name;
		Animation* anim = nullptr;
		bool loop = false;

		Clip() { ; }
		Clip(HashString n, Animation* a, bool l) : name(n), anim(a), loop(l) { ; }
	};

	struct Transition
	{
		HashString origin;
		HashString destiny;
		HashString trigger;

		unsigned blend = 200u;

		Transition() { ; }
		Transition(HashString o, HashString d, HashString t, unsigned b) : 
			origin(o), destiny(d), trigger(t), blend(b) { ; }
	};

	struct Node
	{
		HashString name;
		HashString clipName;

		Node() { ; }
		Node(HashString n, HashString cn) : name(n), clipName(cn) { ; }
	};

	std::vector<Node> nodes;
	std::vector<Transition> transitions;
	std::vector<Clip> clips;
	unsigned defaultNode = 0u;
};

#endif // __STATEMACHINE_H_