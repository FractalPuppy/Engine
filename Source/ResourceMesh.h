#ifndef __ResourceMesh_h__
#define __ResourceMesh_h__

#include "Resource.h"

#include <vector>
#include "Math/float3.h"
#include "Math/float4x4.h"
#include "Geometry/AABB.h"

#define MAX_WEIGHTS_PER_BONE 4

class ComponentRenderer;
class GameObject;

class ResourceMesh : public Resource
{
public:
	ResourceMesh(unsigned uid);
	ResourceMesh(const ResourceMesh& resource);
	virtual ~ResourceMesh();

	// Getters
	inline unsigned GetVAO() { return VAO; };
	inline unsigned GetVBO() { return VBO; };
	inline unsigned GetEBO() { return EBO; };

	bool LoadInMemory() override;
	void DeleteFromMemory() override;

	// Mesh specific
	void LinkBones(GameObject* gameobject);
	void Draw(unsigned shaderProgram) const;
	void DrawBbox(unsigned shaderProgram, const AABB& globalBBOX) const;
	AABB GetBoundingBox() const;
	bool Intersects(const LineSegment& line, float* distance);

	// File in Assets especific
	void Rename(const char* newName) override;
	void Delete() override;

private:

	struct BindBone
	{
		math::float4x4 transform; //Transforms from mesh space to bone space
		std::string name;
		GameObject* go = nullptr;
	};

	struct BoneVertex
	{
		int boneID[MAX_WEIGHTS_PER_BONE] = { 0, 0, 0, 0 };
	};

	struct BoneWeight
	{
		float weight[MAX_WEIGHTS_PER_BONE] = { 0.f, 0.f, 0.f, 0.f };
	};

private:
	void ComputeBBox();
	void ProcessVertexTangent(const float vIndex1, const float vIndex2, const float vIndex3);
	void CalculateTangents();
	void SetMesh(const char* meshData);
	void SetMeshBuffers();
	void SetBboxBuffers();

private:
	unsigned VAO = 0;
	unsigned VBO = 0;
	unsigned EBO = 0;

	AABB boundingBox;
	unsigned VAObox = 0;
	unsigned VBObox = 0;
	unsigned EBObox = 0;


	
public:
	std::string name = "";

	std::vector<math::float3> meshVertices;
	std::vector<math::float3> meshNormals;
	std::vector<math::float3> meshTangents;
	std::vector<float> meshTexCoords;
	std::vector<unsigned> meshIndices;

	std::vector<BoneVertex> bindBoneVertexAttaches;
	std::vector<BoneWeight> bindWeightVertexAttaches;
	std::vector<BindBone> bindBones;
};

#endif __ResourceMesh_h__