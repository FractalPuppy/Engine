#include "Application.h"
#include "ModuleProgram.h"
#include "ModuleTextures.h"

#include "Skybox.h"
#include "Math/float4x4.h"
#include "Geometry/Frustum.h"
#include "GL/glew.h"
#include "Brofiler.h"

#define SKYBOX "Skybox"

Skybox::Skybox()
{
	std::string faces[NUMFACES] =
	{
		"right",
		"left",
		"top",
		"bottom",
		"front",
		"back"
	};
	skybox_cubemap = App->textures->LoadCubeMap(faces);

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

	shader = App->program->GetProgram(SKYBOX);
}


Skybox::~Skybox()
{
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

void Skybox::Draw(const Frustum& frustum) const
{
	PROFILE;
	if (!enabled) return;

	glDepthMask(GL_FALSE);

	glUseProgram(shader->id);
	glBindVertexArray(skyboxVAO);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_cubemap);
	float4x4 model(float4x4::FromTRS(frustum.pos, 
		float4x4::identity, float3::one));
	glUniformMatrix4fv(glGetUniformLocation(shader->id,
		"model"), 1, GL_TRUE, &model[0][0]);

	glUniform1i(glGetUniformLocation(shader->id, "skybox"), 0);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);

	glDepthMask(GL_TRUE);
}
