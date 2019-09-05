#include "ModuleUI.h"

#include "Application.h"
#include "ModuleProgram.h"
#include "ModuleResourceManager.h"
#include "ModuleTextures.h"
#include "ModuleFontLoader.h"
#include "ModuleScene.h"
#include "ModuleTime.h"
#include "MouseController.h"
#include "ModuleScript.h"

#include "GameObject.h"
#include "ComponentTransform2D.h"
#include "ComponentImage.h"
#include "ComponentText.h"
#include "ComponentButton.h"

#include "ResourceTexture.h"

#include "GL/glew.h"
#include <queue>
#include "Math/float4x4.h"
#include "Math/TransformOps.h"
#include "Imgui.h"
#include "JSON.h"

ModuleUI::ModuleUI()
{
}

ModuleUI::~ModuleUI()
{
}



bool ModuleUI::Init(JSON* config)
{
	shader = App->program->GetProgram(shaderFile);

	//Normal
	float quadVertices[16] =
	{
			-0.5f, -0.5f, 0.0f, 0.0f, // bottom left
			-0.5f,  0.5f, 0.0f, 1.0f, // top left 
			 0.5f, -0.5f, 1.0f, 0.0f, // bottom right
			 0.5f,  0.5f, 1.0f, 1.0f  // top right
	};

	//Del reves vert
	float quadVerticesFlipVer[16] =
	{
			-0.5f, -0.5f,  0.0f, 1.0f, // bottom left
			-0.5f,  0.5f, 0.0f, 0.0f, // top left 
			 0.5f, -0.5f, 1.0f, 1.0f, // bottom right
			 0.5f,  0.5f, 1.0f, 0.0f  // top right
	};

	//Del reves hor
	float quadVerticesFlipHor[16] =
	{
			-0.5f, -0.5f,  1.0f, 0.0f, // bottom left
			-0.5f,  0.5f,  1.0f, 1.0f, // top left 
			 0.5f, -0.5f, 0.0f, 0.0f, // bottom right
			 0.5f,  0.5f,  0.0f, 1.0f // top right
	};

	GenerateVAO(VAO, quadVertices);
	GenerateVAO(VAO_FV, quadVerticesFlipVer);
	GenerateVAO(VAO_FH, quadVerticesFlipHor);

	JSON_value* scriptJson = config->GetValue("uiConfig");
	if (scriptJson != nullptr)
	{
		uiCursor = scriptJson->GetString("uiCursor", "GhostGlow.cur");
		gameStandarCursor = scriptJson->GetString("gameStandarCursor", "Glow.cur");
	}

	return true;
}

void ModuleUI::SaveConfig(JSON* config)
{
	JSON_value* scriptJson = config->CreateValue();

	scriptJson->AddString("uiCursor", uiCursor.c_str());
	scriptJson->AddString("gameStandarCursor", gameStandarCursor.c_str());
	config->AddValue("uiConfig", *scriptJson);
}

void ModuleUI::GenerateVAO(unsigned& vao, float quadVertices[16])
{

	unsigned int quadIndices[] =
	{
		0,2,1,
		1,2,3
	};

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(quadVertices[0]), &quadVertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

	//Vertex + UV position
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
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

	std::queue<GameObject*> Q;
	Q.push(App->scene->canvas);
	unsigned count = 0U;

	std::priority_queue<RenderOrdering> renderQ;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	while (!Q.empty())
	{
		GameObject* gameObjectToRender = Q.front();
		Q.pop();

		if (gameObjectToRender->isActive())
		{
			for (Component* comp : gameObjectToRender->components)
			{
				switch (comp->type)
				{
				case ComponentType::Button:
					renderQ.push( RenderOrdering( ((Button*)comp)->uiOrder, count++, comp ));
					break;
				case ComponentType::Image:
					renderQ.push(RenderOrdering(((ComponentImage*)comp)->uiOrder, count++, comp));
					break;
				case ComponentType::Text:
					renderQ.push(  RenderOrdering(((Text*)comp)->uiOrder, count++, comp));
					break;
				}
			}
			for (GameObject* gameObjectChildren : gameObjectToRender->children)
			{
				Q.push(gameObjectChildren);
			}
		}
	}

	isHover = false;
	while (!renderQ.empty())
	{
		RenderOrdering top = renderQ.top();
		Component* comp = top.component;

		switch (comp->type)
		{
		case ComponentType::Button:
		{
			Button* button = (Button*)comp;
			RenderImage(*button->buttonImage, currentWidth, currentHeight);
			RenderImage(*button->highlightedImage, currentWidth, currentHeight);
			RenderImage(*button->pressedImage, currentWidth, currentHeight);
			App->fontLoader->RenderText(*button->text, currentWidth, currentHeight);

			if (button->isHovered && !isItemHover)
			{
				isHover = true;
			}
			break;
		}
		case ComponentType::Image:
			RenderImage(*(ComponentImage*)comp, currentWidth, currentHeight);

			if (((ComponentImage*)comp)->isHovered && !isItemHover)
			{
				isHover = true;
			}
			break;

		case ComponentType::Text:
			App->fontLoader->RenderText(*(Text*)comp, currentWidth, currentHeight);
			break;
		}

		renderQ.pop();
	}

	ManageUiHoveredCursorIcon(isHover);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

void ModuleUI::SetButtonHover(const Button* button)
{
	if (topButton == nullptr || topButton->uiOrder < button->uiOrder)
	{
		topButton = button;
	}
}


void ModuleUI::RenderImage(const ComponentImage& componentImage, int currentWidth, int currentHeight)
{

	if (componentImage.texture == nullptr || componentImage.texture == 0 || !componentImage.enabled)
	{
		return;
	}
	
	if (componentImage.IsMasked())
	{
		int maskThreshold = ((float)componentImage.GetMaskAmount() / 100.f) * MASK_DIVISIONS;
		for (unsigned i = 0u; i < MASK_DIVISIONS; ++i) // Filling from up to down
		{
			if (i < maskThreshold)
			{
				mask[i] = 1.0f;
			}
			else
			{
				mask[i] = 0.0f;
			}
		}
	}
	else
	{
		for (unsigned i = 0u; i < MASK_DIVISIONS; ++i)
		{
			mask[i] = 1.0f;
		}
	}

	glUseProgram(shader->id[0]);

	glUniform1fv(glGetUniformLocation(shader->id[0], "mask"), MASK_DIVISIONS, mask);
	int k = glGetUniformLocation(shader->id[0], "mask");

	glUniform4f(glGetUniformLocation(shader->id[0], "textColor"), componentImage.color.x, componentImage.color.y, componentImage.color.z, componentImage.color.w);
	glUniform1i(glGetUniformLocation(shader->id[0], "vertical"), componentImage.isMaskHorizontal ? 0 : 1);
	glUniform1i(glGetUniformLocation(shader->id[0], "horizontal"), componentImage.isMaskHorizontal ? 1 : 0);

	if (componentImage.flipVertical)
	{
		glBindVertexArray(VAO_FV);
	}
	else if (componentImage.flipHorizontal)
	{
		glBindVertexArray(VAO_FH);
	}
	else
	{
		glBindVertexArray(VAO);
	}

	math::float4x4 projection = math::float4x4::D3DOrthoProjRH(-1.0f, 1.0f, currentWidth, currentHeight);
	math::float4x4 model = math::float4x4::identity;

	Transform2D* transform2D = (Transform2D*)componentImage.gameobject->GetComponentOld(ComponentType::Transform2D);

	if (transform2D != nullptr)
	{
		math::float2 imgPos = transform2D->getPosition();
		math::float2 imgSize = transform2D->getSize();
		math::float3 scale = math::float3(imgSize.x, imgSize.y, 1.0f);
		math::float3 center = math::float3(imgPos.x, imgPos.y, 0.0f);
		model = model.Scale(scale, center);
		model.SetTranslatePart(center);

		glUniformMatrix4fv(glGetUniformLocation(shader->id[0], "model"), 1, GL_TRUE, (const float*)&model);
		glUniformMatrix4fv(glGetUniformLocation(shader->id[0], "projection"), 1, GL_TRUE, (const float*)&projection);

		glActiveTexture(GL_TEXTURE0);
		if (componentImage.videoPlaying)
			glBindTexture(GL_TEXTURE_2D, componentImage.videoTex);
		else
			glBindTexture(GL_TEXTURE_2D, componentImage.texture->gpuID);

		glUniform1i(glGetUniformLocation(shader->id[0], "texture0"), 0);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

	glBindVertexArray(0);
	glUseProgram(0);
}

void ModuleUI::DrawGUI()
{
	ImGui::Checkbox("Draw UI in Scene", &showUIinSceneViewport);

	ImGui::Separator();
	ImGui::Text("UI cursor:");
	char* uiCursorAux = new char[64];
	strcpy_s(uiCursorAux, strlen(uiCursor.c_str()) + 1, uiCursor.c_str());
	ImGui::InputText("uiCursor", uiCursorAux, 64);
	uiCursor = uiCursorAux;
	delete[] uiCursorAux;

	ImGui::Separator();
	ImGui::Text("Game cursor:");
	char* gameStandarCursorAux = new char[64];
	strcpy_s(gameStandarCursorAux, strlen(gameStandarCursor.c_str()) + 1, gameStandarCursor.c_str());
	ImGui::InputText("gameStandarCursor", gameStandarCursorAux, 64);
	gameStandarCursor = gameStandarCursorAux;
	delete[] gameStandarCursorAux;
}

void ModuleUI::ManageUiHoveredCursorIcon(bool isHovered)
{
	if (App->time->gameState == GameState::RUN)
	{
		if (isHovered && changeHoverCursorIcon)
		{
			MouseController::ChangeCursorIcon(uiCursor);
			changeHoverCursorIcon = false;
			changeNotHoverCursorIcon = true;
		}
		else if (!isHovered && changeNotHoverCursorIcon)
		{
			MouseController::ChangeCursorIcon(gameStandarCursor);
			changeHoverCursorIcon = true;
			changeNotHoverCursorIcon = false;
		}
	}
}