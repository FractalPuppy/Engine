#ifndef __FileImporter_h__
#define __FileImporter_h__

#include <map>
#include "Math/float4x4.h"

struct aiScene;
struct aiNode;
struct aiMesh;
struct aiBone;
struct aiAnimation;
class GameObject;
class ResourceMesh;
class ResourceAnimation;

class Resource;
class ResourceModel;

class FileImporter {
public:
	FileImporter();
	~FileImporter();

	void ImportAsset(const char* file, const char* folder);

	void ProcessNode(const std::map<unsigned,unsigned> &meshmap, const aiNode* node, const aiScene* scene, GameObject* parent);

	bool ImportFBX(const char * file, const char* folder, ResourceModel* resource);

	bool ImportScene(const aiScene& scene, const char* file, const char* folder, ResourceModel* resource);
	void ImportAnimation(const aiAnimation& animation, char* data);

	void ImportMesh(const aiMesh& mesh, char* data);

	void ImportBones(aiBone** bones, unsigned numBones, char* data) const;

	unsigned GetMeshSize(const aiMesh& mesh) const;

	unsigned GetAnimationSize(const aiAnimation& animation) const;

};

#endif __FileImporter_h__

