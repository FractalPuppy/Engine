#include "PanelResourcePreview.h"

#include "Application.h"
#include "ModuleResourceManager.h"
#include "ModuleScene.h"

#include "Viewport.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentCamera.h"
#include "ComponentRenderer.h"

#include "Resource.h"
#include "ResourceTexture.h"
#include "ResourceMesh.h"

#include "GL/glew.h"
#include "imgui.h"
#include "Math/float4x4.h"
#include "Math/float3.h"
#include "Geometry/Frustum.h"

#include "ModuleRender.h"
#include "ResourceMaterial.h"
#include "ModuleProgram.h"
#define TEST_UID 3974562548

PanelResourcePreview::PanelResourcePreview()
{
	preview = new Viewport("Preview");
}

PanelResourcePreview::~PanelResourcePreview()
{
	//this->ReleaseResource();

	//if (resource != nullptr) App->resManager->DeleteResource(resource->GetUID()); resource = nullptr;
	delete preview;
	if (gameobject_camera != nullptr) delete gameobject_camera;
	// if (target != nullptr) delete target;
}

void PanelResourcePreview::Init()
{
	preview->CreateFrameBuffer(480.0F, 480.0F);

	gameobject_camera = new GameObject(float4x4::identity,"preview_camera", 0);
	camera = (ComponentCamera*)gameobject_camera->CreateComponent(ComponentType::Camera);
	gameobject_camera->UpdateGlobalTransform();
	
	camera->frustum->pos = float3(0.0f, 0.0f, -200.0f);

	this->SetResource(TEST_UID);
	assert(camera);
}

void PanelResourcePreview::Draw()
{
	if (!ImGui::Begin("Resource Preview", &enabled))
	{
		ImGui::End();
		return;
	}
	
	if (resource == nullptr) 
	{
		// DEBUG
		this->SetResource(TEST_UID);
		//ImGui::End();
		//return;
	}

	switch (resource->GetType())
	{
	case TYPE::TEXTURE:
		DisplayTexture();
		break;

	case TYPE::MESH:
		DisplayMesh();
		break;

	case TYPE::MATERIAL:

		break;

	case TYPE::AUDIO:

		break;

	case TYPE::ANIMATION:

		break;

	default:
		break;
	}


	// ImVec2 region_size = ImGui::GetContentRegionAvail();
	// ImGui::Image((ImTextureID)preview->texture, region_size);

	ImGui::End();
}

void PanelResourcePreview::SetResource(unsigned uid)
{
	resource = App->resManager->Get(uid);
	if (resource == nullptr) return;

	ComponentRenderer* renderer = nullptr;
	switch (resource->GetType())
	{
	case TYPE::MESH:
		target = new GameObject(float4x4::identity, "preview_target", 0);
		renderer = (ComponentRenderer*)target->CreateComponent(ComponentType::Renderer);
		renderer->mesh = (ResourceMesh*)resource;
		target->UpdateBBox();
		camera->frustum->pos = target->bbox.CenterPoint() - 10 * target->bbox.Size().z*float3(0, 0, -1);
		break;
		
	default:
		break;
	}

	LOG("####### Loaded %s", resource->GetFile());
}

void PanelResourcePreview::SetResource(const char* name)
{
	unsigned id = App->resManager->FindByExportedFile(name);
	SetResource(id);
}

void PanelResourcePreview::ReleaseResource()
{
	App->resManager->DeleteResource(resource->GetUID());
	resource = nullptr;
	if (target != nullptr) delete target;
}

void PanelResourcePreview::Render()
{
	glBindFramebuffer(GL_FRAMEBUFFER, preview->FBO);

	ImVec2 region_size = ImGui::GetContentRegionAvail();
	glViewport(0, 0, 480, 480);
	glClearColor(0.8f, 0.8f, 0.8f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glBindBuffer(GL_UNIFORM_BUFFER, App->renderer->GetUBO());
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float4x4), &camera->GetProjectionMatrix()[0][0]);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float4x4), sizeof(float4x4), &camera->GetViewMatrix()[0][0]);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	ComponentRenderer* crenderer = (ComponentRenderer*)target->GetComponent(ComponentType::Renderer);
	ResourceMaterial* material = crenderer->material;
	Shader* shader = material->shader;

	glUseProgram(shader->id);

	material->SetUniforms(shader->id);

	float3 ambientColor = float3::one;
	glUniform3fv(glGetUniformLocation(shader->id,"lights.ambient_color"), 1, (GLfloat*)&ambientColor);
	target->SetLightUniforms(shader->id);

	target->UpdateModel(shader->id);
	crenderer->mesh->Draw(shader->id);

	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PanelResourcePreview::DisplayTexture()
{
	ImVec2 region_size = ImGui::GetContentRegionAvail();
	ResourceTexture* texture = (ResourceTexture*)resource;

	float ratio = texture->height / texture->width;
	if (region_size.y > region_size.x * ratio) region_size.y = region_size.x * ratio;
	else
	{
		float width = region_size.y / ratio;
		ImGui::Dummy( ImVec2( 0.5F*(region_size.x-width), region_size.y ) ); ImGui::SameLine();
		region_size.x = width;
	}

	ImGui::Image((ImTextureID)texture->gpuID, region_size);
}

void PanelResourcePreview::DisplayMesh()
{
	Render();

	ImVec2 region_size = ImGui::GetContentRegionAvail();
	ResourceTexture* texture = (ResourceTexture*)resource;

	float ratio = camera->frustum->AspectRatio();
	if (region_size.y > region_size.x / ratio) region_size.y = region_size.x / ratio;
	else
	{
		float width = region_size.y * ratio;
		ImGui::Dummy(ImVec2(0.5F*(region_size.x - width), region_size.y)); ImGui::SameLine();
		region_size.x = width;
	}

	ImGui::Image((ImTextureID)preview->texture, region_size,ImVec2(0,1),ImVec2(1,0));
}
