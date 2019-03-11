#ifndef __Mesh_h__
#define __Mesh_h__

#include <vector>
#include "Math/float3.h"
#include "Geometry/AABB.h"

class Mesh
{
public:
	Mesh();
	~Mesh();

	void SetMesh(const char* meshData, unsigned uid);
	void Draw(unsigned shaderProgram) const;
	void DrawBbox(unsigned shaderProgram, const AABB &globalBBOX) const;

	AABB GetBoundingBox() const;

	bool Intersects(const LineSegment &line, float* distance);

private:
	void ComputeBBox();
	void ProcessVertexTangent(const float vIndex1, const float vIndex2, const float vIndex3);
	void CalculateTangents();
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
	unsigned UID = 0;
	std::vector<math::float3> meshVertices;
	std::vector<math::float3> meshNormals;
	std::vector<math::float3> meshTangents;
	std::vector<float> meshTexCoords;
	std::vector<unsigned> meshIndices;
};

#endif
