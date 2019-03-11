#ifndef _KDTREE_H
#define _KDTREE_H

#include "MathGeoLib/include/Geometry/AABB.h"
#include "Application.h"
#include "ModuleScene.h"
#include <vector>
#include <unordered_set>
#include "GameObject.h"
//#include "debugdraw.h"
#include <queue>
#include <algorithm>
#include "Brofiler.h"

#define BUCKET_MAX 1024

class KDTNode
{
public:

	~KDTNode();
	void SubDivideAABB(unsigned dimension, float median, KDTNode* left, KDTNode* right) const;

	//members

	KDTNode* upperNode = nullptr;
	unsigned depth = 0u;
	float median = .0f;
	bool isLeaf = false;
	AABB* aabb = nullptr;
	std::vector<GameObject*> bucket;
	unsigned bucketOccupation = 0u;

	KDTNode* leftBranch = nullptr;
	KDTNode* rightBranch = nullptr;

};

class KDTree
{
public:

	~KDTree();

	void Init();
	void Calculate();
	void DebugDraw() const;
	template<typename T>
	void GetIntersections(T &intersector, std::unordered_set<GameObject*> &intersections) const;

	//members

	KDTNode* treeRoot = nullptr;
	bool showOnEditor = true;
	int maxDepth = 6;
	int bucketSize = 5;
};

template<typename T>
void KDTree::GetIntersections(T &intersector, std::unordered_set<GameObject*> &intersections) const
{
	BROFILER_CATEGORY("KDTree intersections", Profiler::Color::Azure);
	std::queue<KDTNode*> Q;
	Q.push(treeRoot);

	while (!Q.empty())
	{
		KDTNode* node = Q.front();
		Q.pop();
		if (node->isLeaf && node->bucketOccupation > 0u && node->aabb->ContainsQTree(intersector)) //check if is not outside
		{
			intersections.insert(node->bucket.begin() + 1, node->bucket.begin() + node->bucketOccupation + 1); //the first is always null due the preincrement on filling the bucket
		}
		if (!node->isLeaf)
		{
			Q.push(node->leftBranch);
			Q.push(node->rightBranch);
		}
	}
}

#endif