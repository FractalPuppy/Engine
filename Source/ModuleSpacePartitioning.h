#ifndef _MODULE_SPACE_PARTITIONING_H
#define _MODULE_SPACE_PARTITIONING_H

#include "Module.h"
#include "KDTree.h"
#include "AABBTree.h"


class ModuleSpacePartitioning :
	public Module
{
public:

	bool Init(JSON * config) override;
	update_status Update(float dt) override;
	bool CleanUp() override;	

	//members

	bool newThreadReady = true;
	unsigned currentKDTree = 0u;
	KDTree kDTree;
	AABBTree aabbTree;
	AABBTree aabbTreeLighting;

};

#endif
