#include "KDTree.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "debugdraw.h"

KDTNode::~KDTNode()
{
	RELEASE(aabb);
	RELEASE(leftBranch);
	RELEASE(rightBranch);
}

void KDTNode::SubDivideAABB(unsigned dimension, float median, KDTNode* left, KDTNode* right) const
{
	float3 centerLeft = aabb->CenterPoint();
	float3 centerRight = aabb->CenterPoint();
	float3 newSizeLeft = aabb->Size(); 
	float3 newSizeRight = aabb->Size(); 

	switch (dimension)
	{
		case 0:
		{
			float xL = centerLeft.x - newSizeLeft.x * 0.5f;
			newSizeLeft.x = median - xL;
			centerLeft.x = xL + (newSizeLeft.x) * 0.5f;

			float xR = centerRight.x + newSizeRight.x * 0.5f;
			newSizeRight.x = xR - median;
			centerRight.x = xR - (newSizeRight.x) * 0.5f;

			break;
		}
		case 1:
		{
			float yL = centerLeft.y - newSizeLeft.y * 0.5f;
			newSizeLeft.y = median - yL;
			centerLeft.y = yL + (newSizeLeft.y) * 0.5f;

			float yR = centerRight.y + newSizeRight.y * 0.5f;
			newSizeRight.y = yR - median;
			centerRight.y = yR - (newSizeRight.y) * 0.5f;

			break;
		}
		case 2:		
		{
			float zL = centerLeft.z - newSizeLeft.z * 0.5f;
			newSizeLeft.z = median - zL;
			centerLeft.z = zL + (newSizeLeft.z) * 0.5f;

			float zR = centerRight.z + newSizeRight.z * 0.5f;
			newSizeRight.z = zR - median;
			centerRight.z = zR - (newSizeRight.z) * 0.5f;

			break;
		}
	}

	left->aabb->SetFromCenterAndSize(centerLeft, newSizeLeft);
	right->aabb->SetFromCenterAndSize(centerRight, newSizeRight);
}


KDTree::~KDTree()
{
	RELEASE(treeRoot);
}
void KDTree::Init()
{
	maxDepth = 6;
	bucketSize = 5;
	LOG("Initializating KDTree with depth %d", maxDepth);
	RELEASE(treeRoot);
	std::queue<KDTNode*> Q;
	treeRoot = new KDTNode();
	treeRoot->aabb = new AABB();
	Q.push(treeRoot);
	while (!Q.empty())
	{
		KDTNode* current = Q.front(); Q.pop();
		current->bucket.resize(BUCKET_MAX);
		if (current->depth <= maxDepth)
		{
			current->leftBranch = new KDTNode();
			current->rightBranch = new KDTNode();
			current->leftBranch->aabb = new AABB();
			current->rightBranch->aabb = new AABB();
			current->leftBranch->depth = current->depth + 1;
			current->rightBranch->depth = current->depth + 1;
			Q.push(current->leftBranch);
			Q.push(current->rightBranch);
		}
		else
			current->isLeaf = true;
	}
}

void KDTree::CleanUp()
{
	if (treeRoot != nullptr)
	{
		RELEASE(treeRoot);
		treeRoot = nullptr;
	}
}

void KDTree::Calculate()
{
	treeRoot->bucketOccupation = 0u;
	treeRoot->isLeaf = false;
	App->scene->GetStaticGlobalAABB(*treeRoot->aabb, treeRoot->bucket, treeRoot->bucketOccupation);
	std::queue<KDTNode*> Q;
	Q.push(treeRoot);
	while (!Q.empty())
	{
		KDTNode* current = Q.front(); Q.pop();
		if (current->bucketOccupation > bucketSize && current->depth <= maxDepth)
		{
			unsigned dimension = current->depth % 3;
			std::sort(current->bucket.begin(), current->bucket.begin() + current->bucketOccupation, [dimension](const GameObject* go1, const GameObject *go2)
			{
				if (go2 == nullptr)
					return false;
				if (go1 == nullptr)
					return true;
				return go1->transform->GetGlobalPosition()[dimension] > go2->transform->GetGlobalPosition()[dimension];
			});
			if (current->bucketOccupation % 2 == 0)
			{
				unsigned middle = (current->bucketOccupation * 0.5f) + 1u; //the first is always null due the preincrement on filling the bucket
				current->median = (current->bucket[middle - 1]->transform->GetGlobalPosition()[dimension] + current->bucket[middle]->transform->GetGlobalPosition()[dimension]) / 2.f;
			}
			else
			{
				unsigned middle = (current->bucketOccupation * 0.5f) + 1u; //the first is always null due the preincrement on filling the bucket
				current->median = current->bucket[middle]->transform->GetGlobalPosition()[dimension];
			}
			current->leftBranch->bucketOccupation = 0u;
			current->rightBranch->bucketOccupation = 0u;

			current->SubDivideAABB(dimension, current->median, current->leftBranch, current->rightBranch);
			Q.push(current->leftBranch);
			Q.push(current->rightBranch);

			for (unsigned i = 1u; i <= current->bucketOccupation; ++i) //the first is always null due the preincrement on filling the bucket
			{
				if (current->leftBranch->aabb->ContainsQTree(current->bucket[i]->bbox))
				{
					current->leftBranch->bucket[++current->leftBranch->bucketOccupation] = current->bucket[i];
				}
				if (current->rightBranch->aabb->ContainsQTree(current->bucket[i]->bbox))
				{
					current->rightBranch->bucket[++current->rightBranch->bucketOccupation] = current->bucket[i];
				}
			}
			current->isLeaf = false;
		}
		else
		{
			current->isLeaf = true;
		}
	}
}

void KDTree::DebugDraw() const
{
	if (treeRoot == nullptr)
		return;
	std::queue<KDTNode*> Q;
	Q.push(treeRoot);

	while (!Q.empty())
	{
		KDTNode* node = Q.front();
		Q.pop();

		if (!node->isLeaf)
		{
			Q.push(node->leftBranch);
			Q.push(node->rightBranch);
		}
		else
			dd::aabb(node->aabb->minPoint, node->aabb->maxPoint, dd::colors::Aquamarine);
	}
	
}
