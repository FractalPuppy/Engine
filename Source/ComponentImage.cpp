#include "ComponentImage.h"

#include "glew-2.1.0/include/GL/glew.h"
#include "Globals.h"
#include "Application.h"
#include "ModuleResourceManager.h"
#include "ModuleUI.h"
#include "ModuleInput.h"
#include "ModuleRender.h"
#include "ModuleFileSystem.h"
#include "ModuleTime.h"

#include "ResourceTexture.h"

#include "HashString.h"
#include "JSON.h"
#include "imgui.h"
#include "Math/float2.h"
#include "Viewport.h"
#include "GameObject.h"
#include "ComponentTransform2D.h"
#include "ComponentButton.h"


#define None "None Selected"

ComponentImage::ComponentImage() : Component(nullptr, ComponentType::Image)
{
	//Refact
	if (textureFiles.size() == 0)
	{
		UpdateTexturesList();
	}
}

ComponentImage::ComponentImage(GameObject* gameobject) : Component(gameobject, ComponentType::Image)
{
	if (textureFiles.size() == 0)
	{
		UpdateTexturesList();
	}
}

ComponentImage::ComponentImage(const ComponentImage &copy) : Component(copy)
{
	if (textureFiles.size() == 0)
	{
		UpdateTexturesList();
	}
	color = copy.color;
	flipHorizontal = copy.flipHorizontal;
	flipVertical = copy.flipVertical;
	if (copy.texture != nullptr)
	{
		texture = (ResourceTexture*)App->resManager->Get(copy.texture->GetUID());
	}
	isMasked = copy.isMasked;
	maskAmount = copy.maskAmount;
	isMaskHorizontal = copy.isMaskHorizontal;
	hoverDetectionMouse1 = copy.hoverDetectionMouse1;
	hoverDetectionMouse3 = copy.hoverDetectionMouse3;
	uiOrder = copy.uiOrder;
	videoPath = copy.videoPath;
	fps = copy.fps;
}

ComponentImage::~ComponentImage()
{
	if (texture != nullptr)
	{
		App->resManager->DeleteResource(texture->GetUID());
		texture = nullptr;
	}
}

Component* ComponentImage::Clone() const
{
	return new ComponentImage(*this);
}

void ComponentImage::DrawProperties()
{
	if (ImGui::CollapsingHeader("Image", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool removed = Component::DrawComponentState();
		if (removed)
		{
			return;
		}
		if (showHoverDetectInEditor) ImGui::InputInt("UI Order##Image", &uiOrder);

		//texture selector
		if (ImGui::BeginCombo("Texture", texture != nullptr ? texture->GetName() : None))
		{
			bool none_selected = (texture == nullptr);
			if (ImGui::Selectable(None, none_selected))
			{
				if (texture != nullptr)
				{
					App->resManager->DeleteResource(texture->GetUID());
					texture = nullptr;
				}
			}
			if (none_selected)
				ImGui::SetItemDefaultFocus();
			for (int n = 0; n < textureFiles.size(); n++)
			{
				bool is_selected = (texture != nullptr && (HashString(texture->GetName()) == HashString(textureFiles[n].c_str())));
				if (ImGui::Selectable(textureFiles[n].c_str(), is_selected) && !is_selected)
				{
					// Delete previous texture
					if (texture != nullptr)
						App->resManager->DeleteResource(texture->GetUID());

					texture = (ResourceTexture*)App->resManager->GetByName(textureFiles[n].c_str(), TYPE::TEXTURE);
				}
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		if (ImGui::Button("Refresh List"))
		{
			UpdateTexturesList();
		}
		if (texture != nullptr)
		{
			ImGui::Image((ImTextureID)texture->gpuID, { 200,200 }, { 0,1 }, { 1,0 });
		}

		//color
		ImGui::ColorEdit4("Image color", (float*)&color);

		ImGui::Checkbox("Flip Vertical", &flipVertical);
		ImGui::Checkbox("Flip Horizontal", &flipHorizontal);
		ImGui::Checkbox("Has mask", &isMasked);
		ImGui::Checkbox("Is horizontal mask?", &isMaskHorizontal);
		ImGui::DragInt("Mask amount %", &maskAmount, 1, 0, 100);
		if (showHoverDetectInEditor)
		{
			ImGui::Checkbox("Hover Detection Mouse1", &hoverDetectionMouse1);
			ImGui::Checkbox("Hover Detection Mouse3", &hoverDetectionMouse3);
		}
		ImGui::Separator();
		ImGui::Text("Video");
		ImGui::Text("Path: Game/Video/");
		ImGui::SameLine();
		const int bufSize = videoPath.size() + 1;
		char buf[256];
		memcpy(buf, videoPath.c_str(), bufSize * sizeof(char));

		if (ImGui::InputText("", buf, 256 * sizeof(char))) 
		{
			videoPath = std::string(buf);
		}		

		if (ImGui::DragFloat("FPS", &fps))
		{
			frameTime = 1.0f / fps;
		}
		if (ImGui::Button("Play video"))
		{
			PlayVideo();
		}
		if (ImGui::Button("Stop video"))
		{
			StopVideo();
		}

		ImGui::Separator();
	}
}

void ComponentImage::UpdateTexturesList()
{
	textureFiles.clear();
	textureFiles = App->resManager->GetResourceNamesList(TYPE::TEXTURE, true);
}

void ComponentImage::Update()
{
	math::float2 mouse = reinterpret_cast<const float2&>(App->input->GetMousePosition());
#ifndef GAME_BUILD
	float screenX = mouse.x - App->renderer->viewGame->winPos.x - (App->ui->currentWidth * .5f);
	float screenY = mouse.y - App->renderer->viewGame->winPos.y - (App->ui->currentHeight * .5f);
#else
	float screenX = mouse.x - (App->ui->currentWidth * .5f);
	float screenY = mouse.y - (App->ui->currentHeight * .5f);
#endif
	Transform2D* rectTransform = gameobject->GetComponent<Transform2D>();
	math::float2 pos = rectTransform->getPosition();
	math::float2 size = rectTransform->getSize();
	float buttonX = pos.x;
	float buttonY = pos.y;

	math::float2 buttonMin = float2(buttonX - size.x *.5f, -buttonY - size.y *.5f);
	math::float2 buttonMax = float2(buttonX + size.x *.5f, -buttonY + size.y *.5f);

	if ((App->ui->GetButtonHover() == nullptr || App->ui->GetButtonHover()->uiOrder <= uiOrder) && 
		screenX > buttonMin.x && screenX < buttonMax.x && 
		screenY > buttonMin.y && screenY < buttonMax.y)
	{
		isHovered = true;
		if (hoverDetectionMouse1) App->ui->uiHoveredMouse1 = true;
		if (hoverDetectionMouse3) App->ui->uiHoveredMouse3 = true;
	}
	else
		isHovered = false;

	if (videoPlaying)
	{
		frameTimer -= App->time->gameDeltaTime;
		if (frameTimer < 0.f) 
		{
			cap >> frame;
			if (!frame.empty()) 
			{
				cv::Mat flipped;
				cv::flip(frame, flipped, 0);
				frameTimer = frameTime;
				if (videoTex == 0u)
					glGenTextures(1, &videoTex);
				glBindTexture(GL_TEXTURE_2D, videoTex);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame.cols, frame.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, flipped.ptr());
			}
			else 
			{
				LOG("Video finished");
				videoFinished = true;
				videoPlaying = false;
			}
		}
	}

}

void ComponentImage::UpdateTexture(std::string textureName)
{
	if (texture != nullptr && textureName != None)
	{
		App->resManager->DeleteResource(texture->GetUID());
		texture = nullptr;
	}
	texture = (ResourceTexture*)App->resManager->GetByName(textureName.c_str(), TYPE::TEXTURE);
}

void ComponentImage::Save(JSON_value *value)const
{
	Component::Save(value);
	value->AddUint("textureUID", (texture != nullptr) ? texture->GetUID() : 0u);
	value->AddString("textureName", texture ? texture->GetName() : "");
	value->AddFloat4("color", color);
	value->AddInt("FlipVertical", flipVertical);
	value->AddInt("FlipHorizontal", flipHorizontal);
	value->AddInt("isMasked", isMasked);
	value->AddInt("maskAmount", maskAmount);
	value->AddInt("isMaskHorizontal", isMaskHorizontal);
	value->AddInt("hoverDetectionMouse1", hoverDetectionMouse1);
	value->AddInt("hoverDetectionMouse3", hoverDetectionMouse3);
	value->AddInt("UIOrder", uiOrder);
	value->AddString("videoPath", videoPath.c_str());
	value->AddFloat("fps", fps);

}

void ComponentImage::Load(JSON_value* value)
{
	Component::Load(value);
	unsigned uid = value->GetUint("textureUID");
	texture = (ResourceTexture*)App->resManager->Get(uid);
	if (!texture) texture = (ResourceTexture*)App->resManager->GetByName(value->GetString("textureName", ""), TYPE::TEXTURE);
	color = value->GetFloat4("color");
	flipVertical = value->GetInt("FlipVertical");
	flipHorizontal = value->GetInt("FlipHorizontal");
	isMasked = value->GetInt("isMasked");
	maskAmount = value->GetInt("maskAmount");
	isMaskHorizontal = value->GetInt("isMaskHorizontal");
	hoverDetectionMouse1 = value->GetInt("hoverDetectionMouse1", 1);
	hoverDetectionMouse3 = value->GetInt("hoverDetectionMouse3", 1);
	uiOrder = value->GetInt("UIOrder", 0);
	videoPath = value->GetString("videoPath", videoPath.c_str());
	fps = value->GetFloat("fps", fps);
}

ENGINE_API void ComponentImage::SetMaskAmount(int maskAmount)
{
	this->maskAmount = maskAmount;
}

int ComponentImage::GetMaskAmount() const
{
	return maskAmount;
}

bool ComponentImage::IsMasked() const
{
	return isMasked;
}

float ComponentImage::PlayVideo()
{
	using namespace cv;	
	cap.open(std::string("../Game/Video/") + videoPath.c_str());
	// Check if video opened successfully
	if (!cap.isOpened()) {
		LOG("Error opening video");
	}
	else
	{
		videoPlaying = true;
		videoFinished = false;
		cap >> frame;
		frameTime = 1.0f / fps;
		frameTimer = 0.f;
	}

	fps = cap.get(cv::CAP_PROP_FPS);
	return int(cap.get(cv::CAP_PROP_FRAME_COUNT)) / fps;
}

void ComponentImage::StopVideo()
{
	videoPlaying = false;
	videoFinished = true;
	frame.release();
	cap.release();
	frameTimer = 0.f;
}
