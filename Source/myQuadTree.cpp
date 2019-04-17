#include "Application.h"

#include "ModuleProgram.h"
#include "GameObject.h"

#include "myQuadTree.h"
#include "Geometry/LineSegment.h"
#include "Math/Quat.h"
#include "Math/float2.h"
#include "Math/float4x4.h"
#include "Math/MathFunc.h"
#include "GL/glew.h"
#include <stack>

myQuadTree::myQuadTree(AABB limits) : limits(limits)
{
	rootIndex = 0;
	nodes.reserve(100);
	AllocateNode(0);
}

myQuadTree::~myQuadTree()
{
	for (auto &node : nodes)
	{
		RELEASE(node);
	}
}


void myQuadTree::Clear(AABB limits)
{
	this->limits = limits;
	for (auto &node : nodes)
	{
		RELEASE(node);
	}
	nodes.clear();
	rootIndex = 0;
	AllocateNode(0);
}

void myQuadTree::Insert(GameObject* gameobject)
{
	AABB bbox = gameobject->GetBoundingBox();

	if (bbox.minPoint.x >= limits.minPoint.x)
	{
		//Fits Left
		if (bbox.maxPoint.x <= limits.maxPoint.x)
		{
			//Fits Left+Right
			if (bbox.minPoint.z >= limits.minPoint.z)
			{
				//Fits Left+Right+Top
				if (bbox.maxPoint.z <= limits.maxPoint.z)
				{
					//Fits root
					Add(gameobject, nodes[rootIndex], limits);
					return;
				}
				else
				{
					//Doesn't fit bot
					ExtendLimitBotRight();
				}
			}
			else
			{
				//Doesn't fit top
				ExtendLimitTopRight();
			}
		}
		else
		{
			//Doesn't fit right
			if (bbox.minPoint.z < limits.minPoint.z)
			{
				//Doesn't fit top
				ExtendLimitTopRight();
			}
			else
			{
				ExtendLimitBotRight();
			}
		}
	}
	else
	{
		//Doesn't fit left
		if (bbox.minPoint.z < limits.minPoint.z)
		{
			//Doesn't fit top
			ExtendLimitTopLeft();
		}
		else
		{
			ExtendLimitBotLeft();
		}
	}
	Insert(gameobject); //Try again now
}

void myQuadTree::Add(GameObject* gameobject, Node* node, AABB bbox)
{
	while (true) //should look depth
	{
		float2 half = float2((bbox.minPoint.x + bbox.maxPoint.x), (bbox.minPoint.z + bbox.maxPoint.z))*0.5f;
		AABB goBBox = gameobject->GetBoundingBox();

		bool left = goBBox.minPoint.x < half.x;
		bool right = goBBox.maxPoint.x > half.x;
		bool top = goBBox.minPoint.z < half.y;
		bool bot = goBBox.maxPoint.z > half.y;

		if (node->IsLeaf() || (left && right) || (top && bot))
		{
			node->gameobjects.push_back(gameobject);
			if (node->gameobjects.size() > bucketSize && node->IsLeaf())
			{
				if (node->height < maxHeight)
				{
					Split(node, bbox);
				}
			}
			return;
		}
		//iterate over children nodes
		if (left)
		{
			bbox.maxPoint.x = half.x;
			if (top)
			{
				bbox.maxPoint.z = half.y;
				node = nodes[node->TopLeftChildIndex()];
			}
			else //bot
			{
				bbox.minPoint.z = half.y;
				node = nodes[node->BottomLeftChildIndex()];
			}
		}
		else //right
		{
			bbox.minPoint.x = half.x;
			if (top)
			{
				bbox.maxPoint.z = half.y;
				node = nodes[node->TopRightChildIndex()];
			}
			else
			{
				bbox.minPoint.z = half.y;
				node = nodes[node->BottomRightChildIndex()];
			}
		}
	}
}

void myQuadTree::Split(Node* leaf, AABB leafAABB)
{
	assert(leaf->IsLeaf());
	leaf->childIndex = AllocateNode(leaf);
	float2 half = float2((leafAABB.minPoint.x + leafAABB.maxPoint.x), 
		(leafAABB.minPoint.z + leafAABB.maxPoint.z))*0.5f;
	
	unsigned i = 0;
	while (i < leaf->gameobjects.size())
	{
		GameObject *go = leaf->gameobjects[i];
		AABB bbox = go->GetBoundingBox();

		bool left = bbox.minPoint.x < half.x;
		bool right = bbox.maxPoint.x > half.x;
		bool top = bbox.minPoint.z < half.y;
		bool bot = bbox.maxPoint.z > half.y;
		
		if ((left && right) || (top && bot))
		{
			++i;
			continue;
		}

		AABB child;
		child.minPoint.y = leafAABB.minPoint.y; //Always the same because we are in a quadtree (2D)
		child.maxPoint.y = leafAABB.maxPoint.y;
		if (left)
		{
			child.minPoint.x = leafAABB.minPoint.x;
			child.maxPoint.x = half.x;
			if (top)
			{
				child.minPoint.z = leafAABB.minPoint.z;
				child.maxPoint.z = half.y;//halfz
				Add(go, nodes[leaf->TopLeftChildIndex()], child);
			}
			else //bot
			{
				child.minPoint.z = half.y;
				child.maxPoint.z = leafAABB.maxPoint.z;
				Add(go, nodes[leaf->BottomLeftChildIndex()], child);
			}
		}
		else //right
		{
			child.minPoint.x = half.x;
			child.maxPoint.x = leafAABB.maxPoint.x;
			if (top)
			{
				child.minPoint.z = leafAABB.minPoint.z;
				child.maxPoint.z = half.y;
				Add(go, nodes[leaf->TopRightChildIndex()], child);
			}
			else
			{
				child.minPoint.z = half.y;
				child.maxPoint.z = leafAABB.maxPoint.z;
				Add(go, nodes[leaf->BottomRightChildIndex()], child);
			}
		}
		std::swap(leaf->gameobjects[i], leaf->gameobjects.back());
		leaf->gameobjects.pop_back();
	}
}

void myQuadTree::Remove(const GameObject & gameobject)
{
	for (const auto& node : nodes)
	{
		if (node->Remove(gameobject))
		{
			return;
		}
	}
}

int myQuadTree::AllocateNode(Node *parent)
{
	int index = nodes.size();
	for (unsigned i = 0; i < 4; i++)
	{
		Node *n = new Node();
		n->parent = parent;
		n->height = parent != nullptr ? parent->height + 1 : 1;
		n->childIndex = 0XFFFFFFFF;
		nodes.push_back(n);
	}
	return index;
}

void myQuadTree::Draw() const
{
	for (const auto& node: nodes)
	{
		Draw(GetBoundingBox(*node));
	}
}

void myQuadTree::Draw(AABB bbox) const
{
	unsigned shader = App->program->defaultShader->id[0];
	glUseProgram(shader);
	GLfloat vertices[] = {
		-0.5, -0.5, -0.5, 1.0,
		0.5, -0.5, -0.5, 1.0,
		0.5,  0.5, -0.5, 1.0,
		-0.5,  0.5, -0.5, 1.0,
		-0.5, -0.5,  0.5, 1.0,
		0.5, -0.5,  0.5, 1.0,
		0.5,  0.5,  0.5, 1.0,
		-0.5,  0.5,  0.5, 1.0,
	};
	GLuint vbo_vertices;
	glGenBuffers(1, &vbo_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLushort elements[] = {
		0, 1, 2, 3,
		4, 5, 6, 7,
		0, 4, 1, 5, 2, 6, 3, 7
	};
	GLuint ibo_elements;
	glGenBuffers(1, &ibo_elements);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_elements);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	float4x4 boxtransform = float4x4::FromTRS(bbox.CenterPoint(), Quat::identity, bbox.Size());
	glUniformMatrix4fv(glGetUniformLocation(shader,
		"model"), 1, GL_TRUE, &(boxtransform)[0][0]);

	float blue[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	glUniform4fv(glGetUniformLocation(shader,
		"Vcolor"), 1, blue);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,  // attribute
		4,                  // number of elements per vertex, here (x,y,z,w)
		GL_FLOAT,           // the type of each element
		GL_FALSE,           // take our values as-is
		0,                  // no extra data between each position
		0                   // offset of first element
	);

	glLineWidth(4.f);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_elements);
	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, 0);
	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, (GLvoid*)(4 * sizeof(GLushort)));
	glDrawElements(GL_LINES, 8, GL_UNSIGNED_SHORT, (GLvoid*)(8 * sizeof(GLushort)));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glLineWidth(1.f);

	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDeleteBuffers(1, &vbo_vertices);
	glDeleteBuffers(1, &ibo_elements);
	glUseProgram(0);
}

void myQuadTree::ExtendLimitTopLeft()
{
	limits.minPoint.x -= limits.maxPoint.x - limits.minPoint.x;
	limits.minPoint.z -= limits.maxPoint.z - limits.minPoint.z;
	RecomputeRoot(QUADRANT::BR); //root becomes bottom right quadrant
}

void myQuadTree::ExtendLimitTopRight()
{
	limits.maxPoint.x += limits.maxPoint.x - limits.minPoint.x;
	limits.minPoint.z -= limits.maxPoint.z - limits.minPoint.z;
	RecomputeRoot(QUADRANT::BL);
}

void myQuadTree::ExtendLimitBotLeft()
{
	limits.minPoint.x -= limits.maxPoint.x - limits.minPoint.x;
	limits.maxPoint.z += limits.maxPoint.z - limits.minPoint.z;
	RecomputeRoot(QUADRANT::TR);
}

void myQuadTree::ExtendLimitBotRight()
{
	limits.maxPoint.x += limits.maxPoint.x - limits.minPoint.x;
	limits.maxPoint.z += limits.maxPoint.z - limits.minPoint.z;
	RecomputeRoot(QUADRANT::TL);
}

void myQuadTree::RecomputeRoot(QUADRANT q)
{
	Node* oldRoot = nodes[rootIndex+(int)q];
	if (q != QUADRANT::TL)
	{
		Swap(nodes[rootIndex], nodes[rootIndex+ (int)q]);

		if (!oldRoot->IsLeaf())
		{
			nodes[oldRoot->TopLeftChildIndex()]->parent = oldRoot;
			nodes[oldRoot->TopRightChildIndex()]->parent = oldRoot;
			nodes[oldRoot->BottomLeftChildIndex()]->parent = oldRoot;
			nodes[oldRoot->BottomRightChildIndex()]->parent = oldRoot;
		}
	}

	int oldRootNodeIndex = rootIndex;
	rootIndex = AllocateNode(0);
	Node *newRoot = nodes[rootIndex];
	newRoot->childIndex = oldRootNodeIndex;
	nodes[newRoot->TopLeftChildIndex()]->parent = newRoot;
	nodes[newRoot->TopRightChildIndex()]->parent = newRoot;
	nodes[newRoot->BottomLeftChildIndex()]->parent = newRoot;
	nodes[newRoot->BottomRightChildIndex()]->parent = newRoot;
}
AABB myQuadTree::GetBoundingBox(const Node &node) const
{
	if (!node.parent)
	{
		return limits;
	}
	AABB aabb = GetBoundingBox(*node.parent);
	float2 half = float2((aabb.minPoint.x + aabb.maxPoint.x), (aabb.minPoint.z + aabb.maxPoint.z))*0.5f;

	int quadrant = 0;
	for (unsigned i = 1; i < 4; i++)
	{
		if (nodes[node.parent->childIndex + i] == &node)
		{
			quadrant = i;
		}
	}
	
	switch ((QUADRANT)quadrant)
	{
	case QUADRANT::TL:
		aabb.maxPoint.x = half.x;
		aabb.maxPoint.z = half.y;
		return aabb;
	case QUADRANT::TR:
		aabb.minPoint.x = half.x;
		aabb.maxPoint.z = half.y;
		return aabb;
	case QUADRANT::BL:
		aabb.maxPoint.x = half.x;
		aabb.minPoint.z = half.y;
		return aabb;
	case QUADRANT::BR:
		aabb.minPoint.x = half.x;
		aabb.minPoint.z = half.y;
		return aabb;
	default:
		assert(false);
		return aabb;
	}
}

std::list<std::pair<float, GameObject*>> myQuadTree::GetIntersections(const LineSegment & line) const
{//for now only returns unsorted list of GO
	std::stack<Node*> stack;
	stack.push(nodes[rootIndex]);
	std::list<std::pair<float, GameObject*>> intersections;
	while (!stack.empty())
	{
		Node *n = stack.top();
		stack.pop();
		AABB bbox = GetBoundingBox(*n); //2D AABB because y coordinate is 0
		bbox.minPoint.y = -FLOAT_INF;
		bbox.maxPoint.y = FLOAT_INF;

		if (line.Intersects(bbox))
		{
			for (const auto& go : n->gameobjects)
			{
				float dNear = -FLOAT_INF;
				float dFar = FLOAT_INF;
				if (line.Intersects(go->GetBoundingBox(), dNear, dFar))
				{
					intersections.push_back(std::pair<float, GameObject*>(dNear, go));
				}
			}
			if (n->childIndex != 0xFFFFFFFF)
			{
				stack.push(nodes[n->TopLeftChildIndex()]);
				stack.push(nodes[n->TopRightChildIndex()]);
				stack.push(nodes[n->BottomLeftChildIndex()]);
				stack.push(nodes[n->BottomRightChildIndex()]);
			}
		}
	}
	intersections.sort();
	return intersections;
}

std::list<GameObject*> myQuadTree::GetIntersections(const Frustum & frustum) const
{
	std::stack<Node*> stack;
	stack.push(nodes[rootIndex]);
	std::list<GameObject*> intersections;
	while (!stack.empty())
	{
		Node *n = stack.top();
		stack.pop();
		AABB bbox = GetBoundingBox(*n); //2D AABB because y coordinate is 0
		bbox.minPoint.y = -FLOAT_INF;
		bbox.maxPoint.y = FLOAT_INF;

		if (frustum.Intersects(bbox))
		{
			for (const auto& go : n->gameobjects)
			{
				if (frustum.Intersects(go->GetBoundingBox()))
				{
					intersections.push_back(go);
				}
			}
			if (n->childIndex != 0xFFFFFFFF)
			{
				stack.push(nodes[n->TopLeftChildIndex()]);
				stack.push(nodes[n->TopRightChildIndex()]);
				stack.push(nodes[n->BottomLeftChildIndex()]);
				stack.push(nodes[n->BottomRightChildIndex()]);
			}
		}
	}
	return intersections;
}