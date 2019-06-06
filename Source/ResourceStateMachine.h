#ifndef __RESOURCESTATEMACHINE_H_
#define __RESOURCESTATEMACHINE_H_

#include "Resource.h"

#include "HashString.h"
#include <vector>

#define MAX_CLIP_NAME 64

class ResourceAnimation;

class ResourceStateMachine : public Resource
{
public:
	ResourceStateMachine(unsigned uid);
	ResourceStateMachine(const ResourceStateMachine& resource);
	virtual ~ResourceStateMachine();

public:
	bool LoadInMemory() override;
	void DeleteFromMemory() override;
	void Delete() override;
	void SaveMetafile(const char* file) const override;
	void LoadConfigFromMeta() override;

	unsigned GetStateMachineSize();
	void SetStateMachine(const char* data);
	void SaveStateMachineData(char* data);
	void Save();

	void AddClip(const HashString name, unsigned UID, bool loop);
	void AddNode(const HashString name,const HashString clipName);
	void AddTransition(const HashString origin, const HashString destiny, const HashString trigger, unsigned blend);

	unsigned FindClip(const HashString name);
	unsigned FindNode(const HashString name);
	unsigned FindTransition(const HashString origin, const HashString trigger);

	//Clips setters and getters
	HashString GetClipName(unsigned index);
	unsigned GetClipResource(unsigned index);
	bool GetClipLoop(unsigned index);
	bool GetClipMustFinish(unsigned index);
	float GetClipSpeed(unsigned index);
	void SetClipName(unsigned index, HashString name);
	void SetClipResource(unsigned index, unsigned UID);
	void SetClipLoop(unsigned index, bool loop);
	void SetClipSpeed(unsigned index, float speed);
	void SetClipMustFinish(unsigned index, bool mustFinish);

	//Transitions setters and getters
	HashString GetTransitionOrigin(unsigned index);
	HashString GetTransitionDestiny(unsigned index);
	HashString GetTransitionTrigger(unsigned index);
	float GetTransitionBlend(unsigned index);
	void SetTransitionOrigin(unsigned index, HashString origin);
	void SetTransitionDestiny(unsigned index, HashString destiny);
	void SetTransitionTrigger(unsigned index, HashString trigger);
	void SetTransitionBlend(unsigned index, float blend);

	void RenameTransitionDueNodeChanged(HashString previous, HashString newName);

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

	void ReceiveTrigger(HashString trigger, float &blend, unsigned &node);

	bool isClipsEmpty() { return clips.empty(); }
	bool isNodesEmpty() { return nodes.empty(); }
	bool isTransitionsEmpty() { return transitions.empty(); }

	unsigned GetDefaultNode() { return defaultNode; }
	void SetDefaultNode(unsigned node) { defaultNode = node; }

private:

	struct Clip
	{
		HashString name;
		unsigned UID = 0u;
		float clipSpeed = 1.0f;
		bool loop = false;
		bool mustFinish = false;

		Clip() { ; }
		Clip(HashString n, unsigned u, bool l, bool f, float sp) : 
			name(n), UID(u), loop(l), mustFinish(f), clipSpeed(sp) { ; }
	};

	struct Transition
	{
		HashString origin;
		HashString destiny;
		HashString trigger;

		float blend = 200.f;

		Transition() { ; }
		Transition(HashString o, HashString d, HashString t, float b) : 
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

#endif // __RESOURCESTATEMACHINE_H_