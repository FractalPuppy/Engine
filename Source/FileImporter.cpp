#include "Application.h"

#include "ModuleTextures.h"
#include "ModuleFileSystem.h"
#include "ModuleScene.h"
#include "ModuleResourceManager.h"

#include "GameObject.h"
#include "ComponentRenderer.h"
#include "ComponentTransform.h"
#include "ComponentAnimation.h"

#include "ResourceMaterial.h"
#include "ResourceMesh.h"

#include "Resource.h"
#include "ResourceModel.h"
#include "ResourceMesh.h"
#include "FileImporter.h"
#include "JSON.h"


#include <assert.h>
#include <stack>
#include "Math/float4x4.h"
#include "assimp/cimport.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/mesh.h"
#include "assimp/material.h"


void AddLog(const char* str, char* userData)
{
	std::string info(str);
	info.pop_back();

	LOG("%s", info.c_str());
}

FileImporter::FileImporter()
{
	struct aiLogStream streamLog;
	streamLog = aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, NULL);
	streamLog.callback = AddLog;
	aiAttachLogStream(&streamLog);
}


FileImporter::~FileImporter()
{
}

void FileImporter::ImportAsset(const char *file, const char *folder)
{
	std::string extension(App->fsystem->GetExtension(file));
	if (extension == FBXEXTENSION || extension == FBXCAPITAL)
	{
		App->resManager->ImportFile(file, folder, TYPE::MODEL);
	}
	else if (extension == PNG || extension == TIF || extension == JPG || extension == TGA)
	{
		App->resManager->ImportFile(file, folder, TYPE::TEXTURE);
	}
	else if (extension == TEXTUREEXT)
	{
		App->fsystem->Copy(folder, TEXTURES, file);
	}
	else if (extension == MESHEXTENSION)
	{
		App->fsystem->Copy(folder, MESHES, file);
	}
	else if (extension == MATERIALEXT)
	{
		App->resManager->ImportFile(file, folder, TYPE::MATERIAL);
	}
	else if (extension == SCENEEXTENSION)
	{
		App->resManager->ImportFile(file, folder, TYPE::SCENE);
	}
	else if (extension == ANIMATIONEXTENSION)
	{
		App->resManager->ImportFile(file, folder, TYPE::ANIMATION);
	}
	else if (extension == STATEMACHINEEXTENSION)
	{
		App->resManager->ImportFile(file, folder, TYPE::STATEMACHINE);
	}
}

bool FileImporter::ImportFBX(const char* fbxfile, const char* folder, ResourceModel* resource)
{
	assert(fbxfile != nullptr);
	if (fbxfile == nullptr) return false;

	std::string file(fbxfile);

	const aiScene* scene = aiImportFile((folder + file).c_str(), aiProcess_Triangulate);
	if (scene != nullptr)
	{
		LOG("Imported FBX %s", fbxfile);
		return ImportScene(*scene, fbxfile, folder, resource);
	}
	LOG("Error importing FBX %s", fbxfile);
	return false;
}

bool FileImporter::ImportScene(const aiScene &aiscene, const char* file, const char* folder, ResourceModel* resource)
{
		GameObject* sceneGO = App->scene->CreateGameObject("Scene", App->scene->root);
	sceneGO->CreateComponent(ComponentType::Transform); // To move all around

	std::stack<aiNode*> stackNode;
	std::stack<GameObject*> stackParent;

	unsigned totalMeshes = 0u;

	std::string path(folder);
	path += file;
	std::string name = App->fsystem->GetFilename(file);
	std::string meta(std::string(path) + METAEXT);

	for (unsigned i = 0; i < aiscene.mRootNode->mNumChildren; ++i) //skip rootnode
	{
		stackNode.push(aiscene.mRootNode->mChildren[i]);
		stackParent.push(sceneGO);
	}

	while (!stackNode.empty())
	{
		aiNode* aNode = stackNode.top(); stackNode.pop();
		GameObject* goNode = new GameObject(aNode->mName.C_Str(), App->scene->GetNewUID());
		stackParent.top()->InsertChild(goNode); stackParent.pop();

		ComponentTransform* nodeTransform = (ComponentTransform*)goNode->CreateComponent(ComponentType::Transform);
		ComponentTransform* nodeParentTransform = (ComponentTransform*)goNode->parent->GetComponentOld(ComponentType::Transform);
		nodeTransform->UpdateTransform();
		nodeTransform->SetLocalTransform(reinterpret_cast<const math::float4x4&>(aNode->mTransformation), nodeParentTransform->global);
		nodeTransform->UpdateGlobalTransform();

		for (unsigned i = 0u; i < aNode->mNumMeshes; ++i)
		{
			aiMesh* aMesh = aiscene.mMeshes[aNode->mMeshes[i]];
			unsigned meshSize = GetMeshSize(*aMesh);
			char* meshData = new char[meshSize];

			ImportMesh(*aMesh, meshData);

			ResourceMesh* mesh = nullptr;
			char* metaData = nullptr;
			if (App->fsystem->Load(meta.c_str(), &metaData) == 0)
			{
				mesh = (ResourceMesh*)App->resManager->CreateNewResource(TYPE::MESH);
				resource->AddMesh(mesh);
			}
			else
			{
				JSON *json = new JSON(metaData);
				JSON_value* meshValue = json->GetValue("Mesh");
				mesh = (ResourceMesh*)App->resManager->CreateNewResource(TYPE::MESH, meshValue->GetUint(("Mesh" + std::to_string(totalMeshes)).c_str()));

				// ResourceMesh was created on .meta of model load, now replace previous resource
				App->resManager->ReplaceResource(mesh->GetUID(), mesh);
			}
			std::string exportedFile(MESHES + std::to_string(mesh->GetUID()) + MESHEXTENSION);
			App->fsystem->Save(exportedFile.c_str(), meshData, meshSize);
			mesh->SetFile(path.c_str());
			mesh->SetExportedFile(exportedFile.c_str());
			//mesh->name = name + "_" + std::to_string(totalMeshes);
			mesh->SetName((name + "_" + std::to_string(totalMeshes)).c_str());
			RELEASE_ARRAY(meshData);
			//App->resManager->AddMesh(mesh);
			GameObject* meshGO = new GameObject("mesh", App->scene->GetNewUID());
			ComponentRenderer* crenderer = (ComponentRenderer*)meshGO->CreateComponent(ComponentType::Renderer);
			crenderer->mesh = mesh;
			meshGO->CreateComponent(ComponentType::Transform);
			ComponentTransform* cTransform = (ComponentTransform*)(meshGO->GetComponentOld(ComponentType::Transform));
			cTransform->SetLocalTransform(nodeTransform->global, math::float4x4::identity);
			sceneGO->InsertChild(meshGO);
			++totalMeshes;
		}

		for (unsigned i = 0u; i < aNode->mNumChildren; ++i)
		{
			stackNode.push(aNode->mChildren[i]);
			stackParent.push(goNode);
		}
	}

	for (unsigned i = 0u; i < aiscene.mNumAnimations; i++)
	{
		sceneGO->isBoneRoot = true;
		char* animationData = nullptr;
		unsigned animationSize = GetAnimationSize(*aiscene.mAnimations[i]);
		animationData = new char[animationSize + 1];
		ImportAnimation(*aiscene.mAnimations[i], animationData);

		sceneGO->CreateComponent(ComponentType::Animation);
		ComponentAnimation* animationComponent = (ComponentAnimation*)sceneGO->GetComponentOld(ComponentType::Animation);

		ResourceAnimation* animation = nullptr;
		char* metaData = nullptr;

		if (App->fsystem->Load(meta.c_str(), &metaData) == 0)
		{
			animation = (ResourceAnimation*)App->resManager->CreateNewResource(TYPE::ANIMATION);
			resource->AddAnimation(animation);
		}
		else
		{
			JSON *json = new JSON(metaData);
			JSON_value* animValue = json->GetValue("Animation");
			animation = (ResourceAnimation*)App->resManager->CreateNewResource(TYPE::ANIMATION, animValue->GetUint(("Animation" + std::to_string(i)).c_str()));

			// ResourceAniamtion was created on .meta of model load, now replace previous resource
			App->resManager->ReplaceResource(animation->GetUID(), animation);
		}
		animationComponent->anim = animation;
		animationData[animationSize] = 0;

		std::string exportedFile(IMPORTED_ANIMATIONS + std::to_string(animation->GetUID()) + ANIMATIONEXTENSION);
		App->fsystem->Save(exportedFile.c_str(), animationData, animationSize);
		animation->SetFile(path.c_str());
		animation->SetExportedFile(exportedFile.c_str());
		//animation->name = name + "_" + std::to_string(i);
		animation->SetName((name + "_" + std::to_string(i)).c_str());

		//App->resManager->AddAnim(animation);

		RELEASE_ARRAY(animationData);
	}
	// Check that scenes folder exists if not create
	if (!App->fsystem->Exists(SCENES))
		App->fsystem->MakeDirectory(SCENES);

	if (!App->fsystem->Exists((SCENES + App->fsystem->GetFilename(file) + SCENEEXTENSION).c_str()))
	{
		App->scene->SaveScene(*sceneGO, App->fsystem->GetFilename(file).c_str(), SCENES);
	}

	aiReleaseImport(&aiscene);

	sceneGO->deleteFlag = true;

	return true;
}

void FileImporter::ImportMesh(const aiMesh& mesh, char* data)
{
	char* cursor = data;

	unsigned ranges[2] = { mesh.mNumFaces * 3, 	mesh.mNumVertices };
	unsigned rangeBytes = sizeof(ranges);
	memcpy(cursor, ranges, rangeBytes);
	cursor += rangeBytes;

	unsigned int verticesBytes = sizeof(float)*mesh.mNumVertices * 3;
	memcpy(cursor, mesh.mVertices, verticesBytes);
	cursor += verticesBytes;

	bool hasNormals = mesh.HasNormals();
	memcpy(cursor, &hasNormals, sizeof(bool));
	cursor += sizeof(bool);

	if (hasNormals)
	{
		unsigned int normalBytes = sizeof(float)*mesh.mNumVertices * 3;
		memcpy(cursor, mesh.mNormals, normalBytes);
		cursor += verticesBytes;
	}

	bool hasTextureCoords = mesh.HasTextureCoords(0);
	memcpy(cursor, &hasTextureCoords, sizeof(bool));
	cursor += sizeof(bool);

	if (hasTextureCoords)
	{
		for (unsigned int i = 0; i < mesh.mNumVertices; i++)
		{
			memcpy(cursor, &mesh.mTextureCoords[0][i].x, sizeof(float));
			cursor += sizeof(float);
			memcpy(cursor, &mesh.mTextureCoords[0][i].y, sizeof(float));
			cursor += sizeof(float);
		}
	}

	for (unsigned int i = 0; i < mesh.mNumFaces; i++)
	{
		aiFace *face = &mesh.mFaces[i];
		assert(face->mNumIndices == 3);
		memcpy(cursor, face->mIndices, sizeof(int) * 3);
		cursor += sizeof(int) * 3;
	}

	bool hasTangents = mesh.HasTangentsAndBitangents();
	memcpy(cursor, &hasTangents, sizeof(bool));
	cursor += sizeof(bool);

	if (hasTangents)
	{
		unsigned int tangentBytes = sizeof(float)*mesh.mNumVertices * 3;
		memcpy(cursor, mesh.mTangents, tangentBytes);
		cursor += verticesBytes;
	}

	//------------------------BONES---------------------------------------
	if (mesh.HasBones())
	{
		memcpy(cursor, &mesh.mNumBones, sizeof(unsigned));
		cursor += sizeof(unsigned);
		ImportBones(mesh.mBones, mesh.mNumBones, cursor);
	}
	else
	{
		unsigned zero = 0u;
		memcpy(cursor, &zero, sizeof(unsigned));
		cursor += sizeof(unsigned);
	}

}

void FileImporter::ImportBones(aiBone** bones, unsigned numBones, char* data)const
{
	char* cursor = data;

	for (unsigned i = 0u; i < numBones; ++i)
	{
		memcpy(cursor, bones[i]->mName.C_Str(), MAX_BONE_NAME_LENGTH);
		cursor += MAX_BONE_NAME_LENGTH;

		math::float4x4 boneOffset = reinterpret_cast<const math::float4x4&>(bones[i]->mOffsetMatrix);


		memcpy(cursor, &boneOffset[0][0], sizeof(math::float4x4));
		cursor += sizeof(math::float4x4);

		memcpy(cursor, &bones[i]->mNumWeights, sizeof(unsigned));
		cursor += sizeof(unsigned);

		for (unsigned j = 0u; j < bones[i]->mNumWeights; ++j)
		{
			memcpy(cursor, &bones[i]->mWeights[j].mVertexId, sizeof(unsigned));
			cursor += sizeof(unsigned);
			memcpy(cursor, &bones[i]->mWeights[j].mWeight, sizeof(float));
			cursor += sizeof(unsigned);
		}
	}
}


void FileImporter::ImportAnimation(const aiAnimation& animation, char* data)
{
	char* cursor = data;

	memcpy(cursor, &animation.mDuration, sizeof(double));
	cursor += sizeof(double);

	memcpy(cursor, &animation.mTicksPerSecond, sizeof(double));
	cursor += sizeof(double);

	memcpy(cursor, &animation.mNumChannels, sizeof(int));
	cursor += sizeof(int);

	for (unsigned j = 0u; j < animation.mNumChannels; j++)
	{
		memcpy(cursor, animation.mChannels[j]->mNodeName.C_Str(), sizeof(char) * MAX_BONE_NAME_LENGTH);  //Name
		cursor += sizeof(char) * MAX_BONE_NAME_LENGTH;

		memcpy(cursor, &animation.mChannels[j]->mNumPositionKeys, sizeof(int));
		cursor += sizeof(int);

		memcpy(cursor, &animation.mChannels[j]->mNumRotationKeys, sizeof(int));
		cursor += sizeof(int);

		//importar longitud array de posiciones e iterar

		for (unsigned i = 0u; i < animation.mChannels[j]->mNumPositionKeys; i++)
		{
			memcpy(cursor, &animation.mChannels[j]->mPositionKeys[i].mValue, sizeof(float) * 3);
			cursor += sizeof(float) * 3;
		}

		//importar longitud array de rotaciones e iterar

		for (unsigned k = 0u; k < animation.mChannels[j]->mNumRotationKeys; k++)
		{
			memcpy(cursor, &animation.mChannels[j]->mRotationKeys[k].mValue, sizeof(math::Quat));
			cursor += sizeof(math::Quat);
		}
	}
}

//TODO: Obtain animations size in order to store their content
unsigned FileImporter::GetAnimationSize(const aiAnimation& animation) const
{
	unsigned size = 0u;

	size += sizeof(double) * 2 + sizeof(int); //Duration, ticks per second and number of channels

	size += sizeof(int);

	for (unsigned j = 0u; j < animation.mNumChannels; j++)
	{
		size += sizeof(char) * MAX_BONE_NAME_LENGTH;

		size += sizeof(int) * 2;

		for (unsigned i = 0u; i < animation.mChannels[j]->mNumPositionKeys; i++)
		{
			size += sizeof(float) * 3;
		}

		for (unsigned i = 0u; i < animation.mChannels[j]->mNumRotationKeys; i++)
		{
			size += sizeof(math::Quat);
		}
	}

	return size;
}

unsigned FileImporter::GetMeshSize(const aiMesh &mesh) const
{
	unsigned size = 0u;
	unsigned int ranges[2] = { mesh.mNumFaces * 3, mesh.mNumVertices };
	size += sizeof(ranges); //numfaces + numvertices
	size += ranges[0] * 3 * sizeof(int); //indices

	size += sizeof(float)*ranges[1] * 3; //vertices
	size += sizeof(bool) * 3; //has normals + has tcoords + has tangents
	if (mesh.HasNormals())
	{
		size += sizeof(float)*ranges[1] * 3;
	}
	if (mesh.HasTextureCoords(0))
	{
		size += sizeof(float)*ranges[1] * 2;
	}
	if (mesh.HasTangentsAndBitangents())
	{
		size += sizeof(float)*ranges[1] * 3;
	}
	if (mesh.HasBones())
	{
		unsigned boneSize = sizeof(char) * MAX_BONE_NAME_LENGTH;
		boneSize += sizeof(unsigned);
		boneSize += sizeof(math::float4x4);
		boneSize += mesh.mNumVertices * (sizeof(unsigned) + sizeof(float));
		size += mesh.mNumBones * boneSize;
	}
	return size;
}

void FileImporter::ProcessNode(const std::map<unsigned, unsigned> &meshmap, const aiNode * node, const aiScene * scene, GameObject* parent)
{
	assert(node != nullptr);
	if (node == nullptr) return;

	GameObject * gameobject = App->scene->CreateGameObject(node->mName.C_Str(), parent);

	aiMatrix4x4 m = node->mTransformation;
	float4x4 transform(m.a1, m.a2, m.a3, m.a4, m.b1, m.b2, m.b3, m.b4, m.c1, m.c2, m.c3, m.c4, m.d1, m.d2, m.d3, m.d4);
	ComponentTransform* t = (ComponentTransform *)gameobject->CreateComponent(ComponentType::Transform);
	t->AddTransform(transform);

	std::vector<GameObject*> gameobjects;
	gameobjects.push_back(gameobject);
	//for (unsigned k = 1; k < node->mNumMeshes; k++) //Splits meshes of same node into diferent gameobjects 
	//{
	//	GameObject *copy = new GameObject(*gameobject);
	//	gameobjects.push_back(copy);
	//	copy->parent = gameobject->parent;
	//	parent->children.push_back(copy);
	//}

	//for (unsigned i = 0; i < node->mNumMeshes; i++)
	//{
	//	ComponentRenderer* crenderer = (ComponentRenderer*)gameobjects[i]->CreateComponent(ComponentType::Renderer);
	//	auto it = meshmap.find(node->mMeshes[i]);
	//	if (it != meshmap.end())
	//	{
	//		RELEASE(crenderer->mesh);
	//		crenderer->mesh = App->resManager->GetMesh(it->second);
	//		gameobjects[i]->UpdateBBox();
	//	}
	//}
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(meshmap, node->mChildren[i], scene, gameobject);
	}

}


