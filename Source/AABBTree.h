#ifndef _AABBTREE_H
#define _AABBTREE_H

#include "MathGeoLib/include/Geometry/AABB.h"
#include <vector>
#include <stack>
#include <unordered_set>
#include "GameObject.h"

#define MAX_AABB_TREE_NODES 4096
#define MAX_NON_STATIC_GAMEOBJECTS 4096
#define FAT_FACTOR	1.1f

class AABBTreeNode
{
public:
	AABBTreeNode* parent = nullptr;
	AABBTreeNode* leftSon = nullptr;
	AABBTreeNode* rightSon = nullptr;

	AABB aabb;
	GameObject* go = nullptr;
	bool isLeaf = false;
	bool isLeft = false;
	
	unsigned color = 0u;
};

class AABBTree
{
public:

	void Init();
	void CleanUp();
	void Reset();
	void InsertGO(GameObject* go);
	void ReleaseNode(AABBTreeNode* node);
	void Draw() const;
	
	inline void ResetReleaseNode(AABBTreeNode* node);
	void RecalculateBoxes(AABBTreeNode* node);
	inline AABBTreeNode* GetFreeNode(AABBTreeNode* parent);
	inline void MakeLeaf(AABBTreeNode *node, AABBTreeNode *parent, GameObject* go, bool isLeft, unsigned color);
	template<typename T>
	void GetIntersections(T &intersector, std::unordered_set<GameObject*> &intersections);

	//members

	AABBTreeNode* treeRoot = nullptr;
	AABBTreeNode** nodesFreePool = nullptr;
	AABBTreeNode** nodesCreatedPool = nullptr; //all created nodes container to cleaning pourposes
	bool showOnEditor = false;
	unsigned lastFreeNode = MAX_AABB_TREE_NODES - 1;
};

template<typename T>
void AABBTree::GetIntersections(T &intersector, std::unordered_set<GameObject*> &intersections) 
{
	BROFILER_CATEGORY("AABBTree intersections", Profiler::Color::Azure);
	std::stack<AABBTreeNode*> S;
	S.push(treeRoot);
	while (!S.empty())
	{
		AABBTreeNode* node = S.top();
		S.pop();		
		if (node->isLeaf && node->aabb.ContainsQTree(intersector)) //check if is not outside
		{
			intersections.insert(node->go);
		}
		if (!node->isLeaf)
		{
			//keep the aabb as small as possible
			node->aabb.SetNegativeInfinity(); 
			if (node->leftSon != nullptr)
				node->aabb.Enclose(node->leftSon->aabb);
			if (node->rightSon != nullptr)
				node->aabb.Enclose(node->rightSon->aabb);

			if (node->leftSon != nullptr && node->leftSon->aabb.ContainsQTree(intersector))
			{
				S.push(node->leftSon);
			}

			if (node->rightSon != nullptr && node->rightSon->aabb.ContainsQTree(intersector))
			{
				S.push(node->rightSon);
			}

		}
	}
}
#endif