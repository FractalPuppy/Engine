#include "ModuleUI.h"

#include "Application.h"
#include "ModuleProgram.h"
#include "ModuleResourceManager.h"
#include "ModuleTextures.h"
#include "ModuleFontLoader.h"

#include "GameObject.h"
#include "ComponentTransform2D.h"
#include "ComponentImage.h"
#include "ComponentText.h"

#include "ResourceTexture.h"

#include "GL/glew.h"
#include "Math/float4x4.h"
#include "Math/TransformOps.h"

ModuleUI::ModuleUI()
{
}

ModuleUI::~ModuleUI()
{
}



bool ModuleUI::Init(JSON* json)
{
	shader = App->program->GetProgram(shaderFile);

	float quadVertices[] =
	{
			-0.5f, -0.5f, 0.0f, 0.0f, // bottom left
			-0.5f,  0.5f, 0.0f, 1.0f, // top left 
			 0.5f, -0.5f, 1.0f, 0.0f, // bottom right
			 0.5f,  0.5f, 1.0f, 1.0f  // top right
	};

	unsigned int quadIndices[] =
	{
		0,2,1,
		1,2,3
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

	//Vertex + UV position
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return true;
}

update_status ModuleUI::Update(float dt)
{
	return UPDATE_CONTINUE;
}

update_status ModuleUI::PostUpdate()
{
	return UPDATE_CONTINUE;
}

bool ModuleUI::CleanUp()
{
	if (shader != nullptr)
	{
		App->resManager->DeleteProgram(shader->file);
		shader = nullptr;
	}
	return true;
}

void ModuleUI::Draw(int currentWidth, int currentHeight)
{
	this->currentHeight = currentHeight;
	this->currentWidth = currentWidth;

	if (shader == nullptr) return;

	for (std::list<ComponentImage*>::iterator it = images.begin(); it != images.end(); ++it)
	{
		if ((*it)->texture != nullptr && (*it)->texture != 0 && (*it)->enabled)
		{
			RenderImage(*(*it), currentWidth, currentHeight);
		}
	}
	for (std::list<ComponentText*>::iterator it = texts.begin(); it != texts.end(); ++it)
	{
		if ((*it)->enabled)
		{
			App->fontLoader->RenderText(*(*it), currentWidth, currentHeight);
		}
	}
}


void ModuleUI::RenderImage(const ComponentImage& componentImage, int currentWidth, int currentHeight)
{
	if (!componentImage.enabled)
	{
		return;
	}
	glUseProgram(shader->id);

	glUniform4f(glGetUniformLocation(shader->id, "textColor"), componentImage.color.x, componentImage.color.y, componentImage.color.z, componentImage.color.w);

	glBindVertexArray(VAO);

	math::float4x4 projection = math::float4x4::D3DOrthoProjRH(-1.0f, 1.0f, currentWidth, currentHeight);
	math::float4x4 model = math::float4x4::identity;

	ComponentTransform2D* transform2D = (ComponentTransform2D*)componentImage.gameobject->GetComponent(ComponentType::Transform2D);

	if (transform2D != nullptr)
	{

		math::float3 scale = math::float3(transform2D->size.x, transform2D->size.y, 1.0f);
		math::float3 center = math::float3(transform2D->position.x, transform2D->position.y, 0.0f);
		model = model.Scale(scale, center);
		model.SetTranslatePart(center);

		glUniformMatrix4fv(glGetUniformLocation(shader->id, "model"), 1, GL_TRUE, (const float*)&model);
		glUniformMatrix4fv(glGetUniformLocation(shader->id, "projection"), 1, GL_TRUE, (const float*)&projection);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, componentImage.texture->gpuID);
		glUniform1i(glGetUniformLocation(shader->id, "texture0"), 0);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}
	
	glBindVertexArray(0);
	glUseProgram(0);
}
