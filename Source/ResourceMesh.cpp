#include "ResourceMesh.h"

#include "Globals.h"
#include "Application.h"
#include "ModuleFileSystem.h"

#include "JSON.h"

#include "GL/glew.h"
#include "Geometry/Triangle.h"
#include "Geometry/LineSegment.h"
#include "Math/MathConstants.h"
#include "Math/float4x4.h"
#include "Math/float2.h"
#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"

ResourceMesh::ResourceMesh(unsigned uid) : Resource(uid, TYPE::MESH)
{
}

ResourceMesh::ResourceMesh(const ResourceMesh& resource) : Resource(resource)
{
	VAO = resource.VAO;
	VBO = resource.VBO;
	EBO = resource.EBO;

	boundingBox = resource.boundingBox;
	VAObox = resource.VAObox;
	VBObox = resource.VBObox;
	EBObox = resource.EBObox;

	meshVertices = resource.meshVertices;
	meshNormals = resource.meshNormals;
	meshTangents = resource.meshTangents;
	meshTexCoords = resource.meshTexCoords;
	meshIndices = resource.meshIndices;
}

ResourceMesh::~ResourceMesh()
{
	Resource::DeleteFromMemory();
	DeleteFromMemory();
}

bool ResourceMesh::LoadInMemory()
{
	char *data = nullptr;
	App->fsystem->Load((MESHES + std::to_string(UID) + MESHEXTENSION).c_str(), &data);	// Load mesh file

	SetMesh(data); //Deallocates data
	SetMeshBuffers();
	SetBboxBuffers();
	++loaded;

	return true;
}

void ResourceMesh::DeleteFromMemory()
{
	Resource::DeleteFromMemory();
	if (VAO != 0)
	{
		glDeleteVertexArrays(1, &VAO);
		VAO = 0u;
	}
	if (VBO != 0)
	{
		glDeleteBuffers(1, &VBO);
		VBO = 0u;
	}
	if (EBO != 0)
	{
		glDeleteBuffers(1, &EBO);
		EBO = 0u;
	}
	if (VAObox != 0)
	{
		glDeleteVertexArrays(1, &VAObox);
		VAObox = 0u;
	}
	if (VBObox != 0)
	{
		glDeleteBuffers(1, &VBObox);
		VBObox = 0u;
	}
	if (EBObox != 0)
	{
		glDeleteBuffers(1, &EBObox);
		EBObox = 0u;
	}
	meshVertices.clear();
	meshNormals.clear();
	meshTangents.clear();
	meshTexCoords.clear();
	meshIndices.clear();
}

void ResourceMesh::SaveMetafile(const char* file) const
{
	std::string filepath;
	filepath.append(file);
	JSON *json = new JSON();
	JSON_value* meta = json->CreateValue();
	struct stat statFile;
	stat(filepath.c_str(), &statFile);
	meta->AddUint("GUID", UID);
	meta->AddUint("timeCreated", statFile.st_ctime);
	meta->AddUint("VBO", VBO);
	meta->AddUint("EBO", EBO);
	meta->AddUint("VAObox", VAObox);
	meta->AddUint("VBObox", VBObox);
	meta->AddUint("EBObox", EBObox);
	json->AddValue("Mesh", *meta);
	filepath += ".meta";
	App->fsystem->Save(filepath.c_str(), json->ToString().c_str(), json->Size());
}

void ResourceMesh::Load(const JSON_value &config)
{
	Resource::Load(config);
	//TODO: Add variables to load
}

void ResourceMesh::Draw(unsigned shaderProgram) const
{
	glBindVertexArray(VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glDrawElements(GL_TRIANGLES, meshIndices.size(), GL_UNSIGNED_INT, 0);

	// We disable VAO
	glBindVertexArray(0);
}

void ResourceMesh::ProcessVertexTangent(const float vIndex1, const float vIndex2, const float vIndex3)
{
	math::float3 tangent;

	math::float2 UV = math::float2(meshTexCoords[vIndex1 * 2], meshTexCoords[vIndex1 * 2 + 1]);
	math::float2 UV1 = math::float2(meshTexCoords[vIndex2 * 2], meshTexCoords[vIndex2 * 2 + 1]);
	math::float2 UV2 = math::float2(meshTexCoords[vIndex3 * 2], meshTexCoords[vIndex3 * 2 + 1]);

	math::float2 deltaUV1 = UV1 - UV;
	math::float2 deltaUV2 = UV2 - UV;

	math::float3 edge1 = meshVertices[vIndex2] - meshVertices[vIndex1];
	math::float3 edge2 = meshVertices[vIndex3] - meshVertices[vIndex1];

	float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
	tangent = tangent.Normalized();

	meshTangents[vIndex1] = meshTangents[vIndex1] + tangent;
}

void ResourceMesh::CalculateTangents()
{
	assert(meshIndices.size() % 3 == 0); //not triangulated
	if (meshTexCoords.size() == 0)
	{
		LOG("Unnable to set tangents - Not found in mesh & the mesh doesn't have texture coordinates.");
		return;
	}
	meshTangents.resize(meshVertices.size(), math::float3::zero);

	for (unsigned i = 0u; i < meshIndices.size(); i += 3) //for all the triangles of the mesh
	{
		//calculate tangents for each vertex of the triangle
		//V1
		unsigned vIndex1 = meshIndices[i];
		unsigned vIndex2 = meshIndices[i + 1];
		unsigned vIndex3 = meshIndices[i + 2];
		ProcessVertexTangent(vIndex1, vIndex2, vIndex3);
		//V2
		vIndex1 = meshIndices[i + 1];
		vIndex2 = meshIndices[i];
		vIndex3 = meshIndices[i + 2];
		ProcessVertexTangent(vIndex1, vIndex2, vIndex3);
		//V3
		vIndex1 = meshIndices[i + 2];
		vIndex2 = meshIndices[i + 1];
		vIndex3 = meshIndices[i];
		ProcessVertexTangent(vIndex1, vIndex2, vIndex3);

	}

	for (unsigned i = 0u; i < meshVertices.size(); ++i)
	{
		meshTangents[i] = meshTangents[i].Normalized();
	}
}


void ResourceMesh::SetMesh(const char * meshData)
{
	assert(meshData != nullptr);
	if (meshData == nullptr) return;

	const char *data = meshData;

	unsigned int numIndices = *(int*)meshData;
	meshData += sizeof(int);

	unsigned int numVertices = *(int*)meshData;
	meshData += sizeof(int);

	float* vertices = (float*)meshData;
	meshData += sizeof(float) * 3 * numVertices;

	bool hasNormals = *(bool*)meshData;
	meshData += sizeof(bool);

	float* normals = nullptr;
	if (hasNormals)
	{
		normals = (float*)meshData;
		int nNormals = sizeof(float) * 3 * numVertices;
		meshData += nNormals;
		meshNormals.resize(numVertices);
		memcpy(&meshNormals[0], normals, nNormals);
	}

	bool hasTexCoords = *(bool*)meshData;
	meshData += sizeof(bool);

	float* texCoords = nullptr;
	if (hasTexCoords)
	{
		texCoords = (float*)meshData;
		int nTCoords = sizeof(float) * 2 * numVertices;
		meshData += nTCoords;
		meshTexCoords.resize(numVertices * 2);
		memcpy(&meshTexCoords[0], texCoords, nTCoords);
	}

	int* indices = (int*)meshData;
	meshData += sizeof(int) * numIndices;

	meshVertices.resize(numVertices);
	meshIndices.resize(numIndices);
	memcpy(&meshVertices[0], vertices, numVertices * sizeof(float) * 3);
	memcpy(&meshIndices[0], indices, numIndices * sizeof(int));

	ComputeBBox();

	RELEASE_ARRAY(data);
}

void ResourceMesh::SetMeshBuffers()
{
	if (meshTangents.size() == 0) //if the mesh don't have tangents -> calculate them
		CalculateTangents();

	unsigned offsetTexCoords = meshVertices.size() * sizeof(math::float3);
	unsigned offsetNormals = offsetTexCoords + (meshTexCoords.size() * sizeof(float) * 2);
	unsigned offsetTangents = offsetNormals + (meshNormals.size() * sizeof(math::float3));

	unsigned totalSize = offsetTangents + (meshTangents.size() * sizeof(math::float3));

	if (VAO == 0)
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
	}
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, totalSize, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, offsetTexCoords, &meshVertices[0]);
	if (meshTexCoords.size() > 0)
		glBufferSubData(GL_ARRAY_BUFFER, offsetTexCoords, sizeof(float) * meshTexCoords.size(), &meshTexCoords[0]);

	if (meshNormals.size() > 0)
		glBufferSubData(GL_ARRAY_BUFFER, offsetNormals, sizeof(math::float3) * meshNormals.size(), &meshNormals[0]);

	if (meshTangents.size() > 0)
		glBufferSubData(GL_ARRAY_BUFFER, offsetTangents, sizeof(math::float3) * meshTangents.size(), &meshTangents[0]);


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * meshIndices.size(), &meshIndices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)offsetTexCoords);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)offsetNormals);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)offsetTangents);


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	if (VAO == 0 || VBO == 0 || EBO == 0)
	{
		LOG("Error sending mesh to GPU");
	}
}

void ResourceMesh::SetBboxBuffers()
{
	// VAO Creation
	if (VAObox == 0)
	{
		glGenVertexArrays(1, &VAObox);
	}
	glBindVertexArray(VAObox);

	// Buffer Creation with vertex data
	if (VBObox == 0)
	{
		glGenBuffers(1, &VBObox);
	}

	GLfloat boxvertices[] = {
		-0.5, -0.5, -0.5, 1.0,
		0.5, -0.5, -0.5, 1.0,
		0.5,  0.5, -0.5, 1.0,
		-0.5,  0.5, -0.5, 1.0,
		-0.5, -0.5,  0.5, 1.0,
		0.5, -0.5,  0.5, 1.0,
		0.5,  0.5,  0.5, 1.0,
		-0.5,  0.5,  0.5, 1.0,
	};
	glBindBuffer(GL_ARRAY_BUFFER, VBObox);
	glBufferData(GL_ARRAY_BUFFER, sizeof(boxvertices), boxvertices, GL_STATIC_DRAW);

	GLushort elements[] = {
		0, 1, 2, 3,
		4, 5, 6, 7,
		0, 4, 1, 5, 2, 6, 3, 7
	};
	if (EBObox == 0)
	{
		glGenBuffers(1, &EBObox);
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBObox);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,  // attribute
		4,                  // number of elements per vertex, here (x,y,z,w)
		GL_FLOAT,           // the type of each element
		GL_FALSE,           // take our values as-is
		0,                  // no extra data between each position
		0                   // offset of first element
	);

	// Disable VAO
	glBindVertexArray(0);
	glDisableVertexAttribArray(0);

	// Disable VBO and EBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void ResourceMesh::DrawBbox(unsigned shader, const AABB &globalBBOX) const
{
	if (globalBBOX.Volume() <= 0) return;
	glUseProgram(shader);

	float4x4 boxtransform = float4x4::FromTRS(globalBBOX.CenterPoint(), Quat::identity, globalBBOX.Size());
	glUniformMatrix4fv(glGetUniformLocation(shader,
		"model"), 1, GL_TRUE, &(boxtransform)[0][0]);

	float green[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
	glUniform4fv(glGetUniformLocation(shader,
		"Vcolor"), 1, green);

	glLineWidth(4.f);
	glBindVertexArray(VAObox);
	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, 0);
	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, (GLvoid*)(4 * sizeof(GLushort)));
	glDrawElements(GL_LINES, 8, GL_UNSIGNED_SHORT, (GLvoid*)(8 * sizeof(GLushort)));
	glLineWidth(1.f);

	// We disable VAO
	glBindVertexArray(0);

	glUseProgram(0);
}

void ResourceMesh::ComputeBBox()
{
	math::float3 min, max;
	min = max = math::float3(meshVertices[0]);

	for (unsigned i = 0u; i < meshVertices.size(); ++i)
	{
		min.x = MIN(min.x, meshVertices[i].x); //x
		min.y = MIN(min.y, meshVertices[i].y); //y
		min.z = MIN(min.z, meshVertices[i].z); //z

		max.x = MAX(max.x, meshVertices[i].x);
		max.y = MAX(max.y, meshVertices[i].y);
		max.z = MAX(max.z, meshVertices[i].z);
	}
	boundingBox.minPoint = min;
	boundingBox.maxPoint = max;
}

AABB ResourceMesh::GetBoundingBox() const
{
	return boundingBox;
}

bool ResourceMesh::Intersects(const LineSegment &line, float* distance)
{
	bool intersects = false;
	*distance = FLOAT_INF;

	for (unsigned i = 0u; i < meshIndices.size(); i += 3u) //foreach triangle
	{
		math::float3 v0(meshVertices[meshIndices[i]]);
		math::float3 v1(meshVertices[meshIndices[i + 1]]);
		math::float3 v2(meshVertices[meshIndices[i + 2]]);
		Triangle triangle(v0, v1, v2);
		float dist = -1.f;
		if (line.Intersects(triangle, &dist, nullptr))
		{
			intersects = true;
			*distance = MIN(*distance, dist);
		}
	}
	return intersects;
}