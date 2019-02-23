#include "AABBTree.h"
#include "Application.h"
#include "Application.h"
#include "ModuleScene.h"
#include "ComponentLight.h"
#include "Globals.h"

void AABBTree::Init()
{
	LOG("AABBTree Init");
	nodesFreePool = new AABBTreeNode*[MAX_AABB_TREE_NODES];
	nodesCreatedPool = new AABBTreeNode*[MAX_AABB_TREE_NODES];
	for (unsigned i = 0u; i < MAX_AABB_TREE_NODES; ++i)
	{
		nodesCreatedPool[i] = nodesFreePool[i] = new AABBTreeNode();
	}
	treeRoot = GetFreeNode(nullptr);
	treeRoot->isLeaf = false;
}

void AABBTree::CleanUp()
{
	if (nodesCreatedPool != nullptr)
	{
		for (unsigned i = 0u; i < MAX_AABB_TREE_NODES; ++i)
			RELEASE(nodesCreatedPool[i]);
	}
	RELEASE_ARRAY(nodesFreePool);
	RELEASE_ARRAY(nodesCreatedPool);
	lastFreeNode = MAX_AABB_TREE_NODES - 1;
}


void AABBTree::Reset()
{
	CleanUp();
	Init();
}

void AABBTree::InsertGO(GameObject* go)
{	
	assert(go != nullptr); //tried to insert a null GameObject in the AABBTree
	
	std::stack<AABBTreeNode*> S;
	
	//tree root behaves different it could not be binary

	if ((treeRoot->leftSon == nullptr && treeRoot->rightSon != nullptr)
		|| (treeRoot->leftSon == nullptr && treeRoot->rightSon == nullptr))
	{
		MakeLeaf(treeRoot->leftSon, treeRoot, go, true, treeRoot->color + 1);
	}
	else if (treeRoot->leftSon != nullptr && treeRoot->rightSon == nullptr)
	{
		MakeLeaf(treeRoot->rightSon, treeRoot, go, false, treeRoot->color + 1);
	}
	else
	{
		AABB tempLeft = AABB(treeRoot->leftSon->aabb);
		tempLeft.Enclose(go->bbox);
		AABB tempRight = AABB(treeRoot->rightSon->aabb);
		tempRight.Enclose(go->bbox);
		if (tempLeft.Size().Length() < tempRight.Size().Length()) //heuristic to keep the tree as spatial balanced as possible
			S.push(treeRoot->leftSon);
		else
			S.push(treeRoot->rightSon);
	}

	//Binary trees always if enters the while

	while (!S.empty())
	{
		AABBTreeNode* node = S.top();
		S.pop();
		if (node->isLeaf)
		{
			AABBTreeNode* newNode = GetFreeNode(node->parent); //new no leaf node
			newNode->color = node->color;
			newNode->isLeft = node->isLeft;
			if (node->isLeft)
				node->parent->leftSon = newNode;
			else
				node->parent->rightSon = newNode;


			newNode->rightSon = node; //put the old leaf on the right & create a new leaf on the left with the new GO
			newNode->rightSon->isLeft = false;
			newNode->rightSon->color = newNode->color + 1;
			node->parent = newNode;
			MakeLeaf(newNode->leftSon, newNode, go, true, newNode->color + 1);
			RecalculateBoxes(newNode);
		}
		else
		{
			AABB tempLeft = AABB(node->leftSon->aabb);
			tempLeft.Enclose(go->bbox);
			AABB tempRight = AABB(node->rightSon->aabb);
			tempRight.Enclose(go->bbox);
			if (tempLeft.Size().Length() < tempRight.Size().Length()) //heuristic to keep the tree as balanced as possible
				S.push(node->leftSon);
			else
				S.push(node->rightSon);
		}
	}
}

void AABBTree::Draw() const
{
	//Blocked by: https://trello.com/c/eM3mfc3I/1-opengl-directmode-to-debugdraw
	/*
	static ddVec3 colors[6] = { dd::colors::AliceBlue, dd::colors::BlueViolet, dd::colors::Crimson, dd::colors::DarkOliveGreen, dd::colors::DarkViolet, dd::colors::GhostWhite };
	std::stack<AABBTreeNode*> S;
	S.push(treeRoot);
	while (!S.empty())
	{
		AABBTreeNode* node = S.top();
		S.pop();

		ddVec3 color = colors[(node->color) % 6];
		if (node->parent != nullptr)
		{
			dd::line(node->aabb.CenterPoint(), node->parent->aabb.CenterPoint(), color);
		}
		if (node->leftSon != nullptr)
			S.push(node->leftSon);
		if (node->rightSon != nullptr)
			S.push(node->rightSon);

		dd::aabb(node->aabb.minPoint, node->aabb.maxPoint, color);

	}
	*/
}
void AABBTree::RecalculateBoxes(AABBTreeNode* node)
{
	std::stack<AABBTreeNode*> S2;
	S2.push(node);
	while (!S2.empty())
	{
		node = S2.top();
		S2.pop();
		if (node->isLeaf && node != treeRoot)
		{
			S2.push(node->parent);
		}
		else
		{
			node->aabb.SetNegativeInfinity();
			node->aabb.Enclose(node->leftSon->aabb);
			node->aabb.Enclose(node->rightSon->aabb);
			if (node->parent != nullptr)
				S2.push(node->parent);
		}
	}
}
void AABBTree::ReleaseNode(AABBTreeNode* node)
{
	assert(lastFreeNode < MAX_AABB_TREE_NODES); 
	assert(node != nullptr);
	
	if (node->parent == treeRoot) 
	{
		node->go->treeNode = nullptr;
		if (node->isLeft)
			node->parent->leftSon = nullptr;
		else
			node->parent->rightSon = nullptr;	
		nodesFreePool[++lastFreeNode] = node;
		return;
	}
	assert(node->parent->leftSon != nullptr && node->parent->rightSon != nullptr); //not binary!

	AABBTreeNode* brother;
	AABBTreeNode* siblingRoot = node->parent;

	if (node->isLeft)
	{
		brother = siblingRoot->rightSon;
	}
	else
	{
		brother = siblingRoot->leftSon;
	}
	
	brother->isLeft = siblingRoot->isLeft;
	brother->parent = siblingRoot->parent;

	if (brother->isLeft)
		brother->parent->leftSon = brother;
	else
		brother->parent->rightSon = brother;

	nodesFreePool[++lastFreeNode] = siblingRoot;
	nodesFreePool[++lastFreeNode] = node; 
	
}
void AABBTree::ResetReleaseNode(AABBTreeNode* node)
{
	assert(lastFreeNode < MAX_AABB_TREE_NODES);
	assert(node != nullptr);

	nodesFreePool[++lastFreeNode] = node;

}
inline AABBTreeNode* AABBTree::GetFreeNode(AABBTreeNode* parent)
{
	assert(lastFreeNode > 0u); //out of free nodes
	nodesFreePool[lastFreeNode]->parent = parent;
	nodesFreePool[lastFreeNode]->isLeaf = false;
	nodesFreePool[lastFreeNode]->isLeft = false;
	nodesFreePool[lastFreeNode]->go = nullptr;
	nodesFreePool[lastFreeNode]->leftSon = nullptr;
	nodesFreePool[lastFreeNode]->rightSon = nullptr;
	return nodesFreePool[lastFreeNode--];
}

inline void AABBTree::MakeLeaf(AABBTreeNode * node, AABBTreeNode *parent, GameObject* go, bool isLeft, unsigned color)
{
	node = GetFreeNode(parent);
	node->aabb.SetFromCenterAndSize(go->bbox.CenterPoint(), go->bbox.Size() * FAT_FACTOR);
	node->go = go;
	go->treeNode = node;
	node->isLeaf = true;
	node->isLeft = isLeft;
	if (isLeft)
		parent->leftSon = node;
	else
		parent->rightSon = node;

	node->color = color;
}
