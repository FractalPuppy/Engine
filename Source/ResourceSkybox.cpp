#include "ResourceSkybox.h"

#include "Application.h"
#include "ModuleTextures.h"
#include "ModuleResourceManager.h"
#include "ModuleFileSystem.h"
#include "ModuleProgram.h"

#include "ResourceTexture.h"

#include "Brofiler.h"
#include "Math/float4x4.h"
#include "Geometry/Frustum.h"
#include "GL/glew.h"
#include "JSON.h"

#define SKYBOX_SHADER "Skybox"

ResourceSkybox::ResourceSkybox(unsigned uid) : Resource(uid, TYPE::SKYBOX)
{
	shader = App->program->GetProgram(SKYBOX_SHADER);
}

ResourceSkybox::ResourceSkybox(const ResourceSkybox& resource) : Resource(resource)
{
	enabled = resource.enabled;
	for (unsigned int i = 0; i < NUMFACES; ++i)
	{
		textures[i] = resource.textures[i];
	}
	shader = resource.shader;

	skybox_cubemap = resource.skybox_cubemap;
	skyboxVAO = resource.skyboxVAO;
	skyboxVBO = resource.skyboxVBO;
}

ResourceSkybox::~ResourceSkybox()
{
	//DeleteFromMemory();
}

bool ResourceSkybox::LoadInMemory()
{
	skybox_cubemap = LoadCubeMap(textures);

	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	++loaded;

	return true;
}

void ResourceSkybox::DeleteFromMemory()
{
	Resource::DeleteFromMemory();

	//Delete used textures
	for (unsigned int i = 0; i < NUMFACES; ++i)
	{
		if (textures[i] != nullptr)
			textures[i]->DeleteFromMemory();
	}

	if (skybox_cubemap != 0)
	{
		glDeleteTextures(1, &skybox_cubemap);
	}
	if (skyboxVAO != 0)
	{
		glDeleteBuffers(1, &skyboxVAO);
	}
	if (skyboxVBO != 0)
	{
		glDeleteBuffers(1, &skyboxVBO);
	}
}

void ResourceSkybox::SetTextures(std::string faces[NUMFACES])
{
	for (unsigned int i = 0; i < NUMFACES; ++i)
	{
		textures[i] = (ResourceTexture*)App->resManager->GetWithoutLoad(faces[i].c_str());
		if (textures[i] != nullptr)
		{
			textures[i]->SetImageType(IMAGE_TYPE::CUBEMAP);
			textures[i]->SetCubemapIndex(i);
		}
	}
}

void ResourceSkybox::Draw(const Frustum& frustum, bool isEditor) const
{
	PROFILE;
	if (!enabled) return;

	glDepthMask(GL_FALSE);
	if (isEditor)
	{
		glUseProgram(shader->id[1]);
	}
	else
	{
		glUseProgram(shader->id[0]);
	}

	glBindVertexArray(skyboxVAO);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_cubemap);
	float4x4 model(float4x4::FromTRS(frustum.pos,
		float4x4::identity, float3::one));
	glUniformMatrix4fv(glGetUniformLocation(shader->id[0],
		"model"), 1, GL_TRUE, &model[0][0]);

	glUniform1i(glGetUniformLocation(shader->id[0], "skybox"), 0);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);

	glDepthMask(GL_TRUE);
}

unsigned ResourceSkybox::LoadCubeMap(ResourceTexture* faces[]) const
{
	unsigned textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	for (unsigned int i = 0; i < NUMFACES; ++i)
	{
		if (faces[i] != nullptr)
		{
			faces[i]->SetReferences(faces[i]->GetReferences() + 1u);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}