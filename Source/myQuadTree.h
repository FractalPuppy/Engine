#ifndef __myQuadTree_h__
#define __myQuadTree_h__

#include "Geometry/AABB.h"
#include "Geometry/Frustum.h"
#include <vector>

enum class QUADRANT
{
	TL = 0, //Top Left
	TR,
	BL,
	BR
};

class GameObject;

struct Node
{
	Node *parent = nullptr;
	unsigned height = 0;
	unsigned childIndex = 0xFFFFFFFF; //Leaf by default

	bool IsLeaf() const { return childIndex == 0xFFFFFFFF; }

	unsigned TopLeftChildIndex() const { return childIndex; }
	unsigned TopRightChildIndex() const { return childIndex + 1; }
	unsigned BottomLeftChildIndex() const { return childIndex + 2; }
	unsigned BottomRightChildIndex() const { return childIndex + 3; }

	std::vector<GameObject*> gameobjects;

	bool Remove(const GameObject& gameobject)
	{
		for (std::vector<GameObject*>::const_iterator it = gameobjects.begin(); it != gameobjects.end(); ++it)
		{
			if (*it == &gameobject)
			{
				gameobjects.erase(it);
				return true;
			}
		}
		return false;
	}
};

class myQuadTree
{
public:
	myQuadTree(AABB limits);
	~myQuadTree();

	void Clear(AABB limits);
	void Insert(GameObject* gameobject);
	void Add(GameObject* gameobject, Node * node, AABB boundingBox);
	void Split(Node * leaf, AABB leafAABB);
	void Remove(const GameObject& gameobject);

	int AllocateNode(Node * parent);
	void Draw() const;
	void Draw(AABB bbox) const;
	void ExtendLimitTopLeft();
	void ExtendLimitTopRight();
	void ExtendLimitBotLeft();
	void ExtendLimitBotRight();
	void RecomputeRoot(QUADRANT q);
	AABB GetBoundingBox(const Node &node) const;
	std::list<std::pair<float, GameObject*>> GetIntersections(const LineSegment& line) const;
	std::list<GameObject*> GetIntersections(const Frustum & frustum) const;

public:
	unsigned bucketSize = 2;
	unsigned maxHeight = 4;
private:
	std::vector<Node*> nodes;
	AABB limits;
	unsigned rootIndex = -1;
};

#endif __myQuadTree_h__
