#ifndef __ResourceMesh_h__
#define __ResourceMesh_h__

#include "Resource.h"

#include <vector>
#include "Math/float3.h"
#include "Geometry/AABB.h"

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
	void SaveMetafile(const char* file)const override;
	void Load(const JSON_value &config) override;

	// Mesh specific
	void Draw(unsigned shaderProgram) const;
	void DrawBbox(unsigned shaderProgram, const AABB& globalBBOX) const;
	AABB GetBoundingBox() const;
	bool Intersects(const LineSegment& line, float* distance);

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
	std::vector<math::float3> meshVertices;
	std::vector<math::float3> meshNormals;
	std::vector<math::float3> meshTangents;
	std::vector<float> meshTexCoords;
	std::vector<unsigned> meshIndices;
};

#endif __ResourceMesh_h__