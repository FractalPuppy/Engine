#ifndef __ResourceMesh_h__
#define __ResourceMesh_h__

#include "Resource.h"

#include <vector>
#include "Math/float3.h"
#include "Math/float4x4.h"
#include "Geometry/AABB.h"

#define MAX_WEIGHTS_PER_BONE 4
#define DRAW_GL_TRIANGLES 0x004
#define DRAW_GL_STRIP 0x005

class ComponentRenderer;
class GameObject;

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

class ResourceMesh : public Resource
{
public:
	ResourceMesh(unsigned uid);
	ENGINE_API ResourceMesh(const ResourceMesh& resource);
	ResourceMesh(unsigned nVertices, float* vertices, unsigned nIndexes, int* indexes, unsigned nUVs, float* UVs); //This constructor is used by volumetric lights | Assume triangle strip drawing method
	virtual ~ResourceMesh();

	// Getters
	inline unsigned GetVAO() { return VAO; };
	inline unsigned GetVBO() { return VBO; };
	inline unsigned GetEBO() { return EBO; };

	bool LoadInMemory() override;
	void DeleteFromMemory() override;

	void SaveMetafile(const char* file) const override;
	void LoadConfigFromMeta() override {};
	void LoadConfigFromLibraryMeta() override;

	// Mesh specific
	void LinkBones(GameObject* gameobject);
	void Draw(unsigned shaderProgram) const;
	void DrawBbox(unsigned shaderProgram, const math::AABB& globalBBOX) const;
	math::AABB GetBoundingBox() const;
	bool Intersects(const LineSegment& line, float* distance, math::float3* intersectionPoint = nullptr);

	// File in Assets especific
	void Delete() override;

private:
	void ComputeBBox();
	void ProcessVertexTangent(const float vIndex1, const float vIndex2, const float vIndex3);
	void CalculateTangents();
	void SetMesh(const char* meshData);
	void SetMeshBuffers();
	void SetBboxBuffers();

public:
	unsigned VAO = 0u;
	unsigned VBO = 0u;
	unsigned EBO = 0u;

	math::AABB boundingBox;
	unsigned VAObox = 0u;
	unsigned VBObox = 0u;
	unsigned EBObox = 0u;

	
public:

	std::vector<math::float3> meshVertices;
	std::vector<math::float3> meshNormals;
	std::vector<math::float3> meshTangents;
	std::vector<float> meshTexCoords;
	std::vector<unsigned> meshIndices;

	std::vector<BoneVertex> bindBoneVertexAttaches;
	std::vector<BoneWeight> bindWeightVertexAttaches;
	std::vector<BindBone> bindBones;

	int drawingMode = DRAW_GL_TRIANGLES;
};

#endif __ResourceMesh_h__
