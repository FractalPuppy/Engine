#include "Application.h"

#include "ModuleRender.h"
#include "ModuleCamera.h"
#include "ModuleScene.h"

#include "GameObject.h"
#include "ComponentCamera.h"
#include "PanelView.h"

#include "imgui.h"
#include "ImGuizmo.h"
#include "Math/float4x4.h"
#include "GL/glew.h"

PanelView::PanelView()
{
}


PanelView::~PanelView()
{
	if (FBO != 0)
	{
		glDeleteFramebuffers(1, &FBO);
	}
	if (RBO != 0)
	{
		glDeleteRenderbuffers(1, &RBO);
	}
	if (texture != 0)
	{
		glDeleteTextures(1, &texture);
	}
	if (MSAAFBO != 0)
	{
		glDeleteFramebuffers(1, &MSAAFBO);
	}
	if (MSAADEPTH != 0)
	{
		glDeleteRenderbuffers(1, &MSAADEPTH);
	}
	if (MSAACOLOR != 0)
	{
		glDeleteRenderbuffers(1, &MSAACOLOR);
	}
}

void PanelView::Draw()
{
	ImGui::Begin("Scene", &enabled, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	ImVec2 size = ImGui::GetWindowSize();
	//ImGui::SetCursorPos({ 0,0 });

	App->camera->editorcamera->SetAspect(size.x / size.y);
	CreateFrameBuffer(size.x, size.y);
	current_width = size.x;
	current_height = size.y;

	
	//App->renderer->;
	

	ImGui::Image((ImTextureID)texture,
		{ size.x, size.y }, { 0,1 }, { 1,0 });
	////ImGui::SetCursorPos({ -(App->window->width - size.x) / 2,-(App->window->height - size.y) / 2 });

	//focus = ImGui::IsWindowFocused();

	//ImVec2 pos = ImGui::GetWindowPos();
	//ImGuizmo::SetRect(pos.x, pos.y, size.x, size.y);
	//ImGuizmo::SetDrawlist();

	//static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);

	//ImGui::SetCursorPos(ImGui::GetWindowContentRegionMin());

	//if (ImGui::Button("Translate"))
	//{
	//	mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	//}
	//ImGui::SameLine();
	//if (ImGui::Button("Rotate"))
	//{
	//	mCurrentGizmoOperation = ImGuizmo::ROTATE;
	//}
	//ImGui::SameLine();
	//if (ImGui::Button("Scale"))
	//{
	//	mCurrentGizmoOperation = ImGuizmo::SCALE;
	//}

	//if (App->scene->selected != nullptr) //TODO: WORLD translation + rotation, scale? -> LOCAL do it with local !watch rotation
	//{

	//	float4x4 model = App->scene->selected->GetGlobalTransform();
	//	float4x4 view = App->camera->editorcamera->GetViewMatrix();
	//	float4x4 proj = App->camera->editorcamera->GetProjectionMatrix();
	//	ImGuizmo::SetOrthographic(false);
	//	model.Transpose();
	//	//ImGuizmo::DrawCube((float*)&view, (float*)&proj, (float*)&model);
	//	ImGuizmo::Manipulate((float*)&view, (float*)&proj, mCurrentGizmoOperation, ImGuizmo::LOCAL, (float*)&model, NULL, NULL, NULL, NULL);
	//	model.Transpose();
	//		
	//	App->scene->selected->SetGlobalTransform(model);
	//}
	ImGui::End();
}

void PanelView::CreateFrameBuffer(int width, int height)
{
	//CleanUp(); //Delete old FBO,RBO and texture
	if (current_width != width || current_height != height)
	{
		if (FBO == 0)
		{
			glGenFramebuffers(1, &FBO);
			glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		}

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL
		);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0
		);

		glBindTexture(GL_TEXTURE_2D, 0);

		glGenRenderbuffers(1, &RBO);
		glBindRenderbuffer(GL_RENDERBUFFER, RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			LOG("Framebuffer ERROR");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		if (App->renderer->msaa)
		{
			CreateMSAABuffers(width, height);
		}
	}
}

void PanelView::CreateMSAABuffers(int width, int height)
{
	glGenFramebuffers(1, &MSAAFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, MSAAFBO);

	glGenRenderbuffers(1, &MSAADEPTH);
	glBindRenderbuffer(GL_RENDERBUFFER, MSAADEPTH);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, App->renderer->msaa_level, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, MSAADEPTH);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glGenRenderbuffers(1, &MSAACOLOR);
	glBindRenderbuffer(GL_RENDERBUFFER, MSAACOLOR);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, App->renderer->msaa_level, GL_RGBA, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, MSAACOLOR);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		LOG("Framebuffer ERROR");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
