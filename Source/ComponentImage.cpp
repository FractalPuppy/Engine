#include "ComponentImage.h"

#include "Globals.h"
#include "Application.h"
#include "ModuleResourceManager.h"
#include "ModuleUI.h"
#include "ModuleInput.h"
#include "ModuleRender.h"

#include "ResourceTexture.h"

#include "HashString.h"
#include "JSON.h"
#include "imgui.h"
#include "MathGeoLib/include/Math/float2.h"
#include "Viewport.h"
#include "GameObject.h"
#include "ComponentTransform2D.h"

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
	texture = (ResourceTexture*)App->resManager->Get(copy.texture->GetUID());
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
	/*if (movable)
	{
		math::float2 mouse = reinterpret_cast<const float2&>(App->input->GetMousePosition());
		float screenX = mouse.x - App->renderer->viewGame->winPos.x - (App->ui->currentWidth * .5f);
		float screenY = mouse.y - App->renderer->viewGame->winPos.y - (App->ui->currentHeight * .5f);
		Transform2D* rectTransform = gameobject->GetComponent<Transform2D>();
		math::float2 pos = rectTransform->getPosition();
		math::float2 size = rectTransform->getSize();
		float buttonX = pos.x;
		float buttonY = pos.y;

		math::float2 buttonMin = float2(buttonX - size.x *.5f, -buttonY - size.y *.5f);
		math::float2 buttonMax = float2(buttonX + size.x *.5f, -buttonY + size.y *.5f);

		if (screenX > buttonMin.x && screenX < buttonMax.x && screenY > buttonMin.y && screenY < buttonMax.y)
			isHovered = true;
		else
			isHovered = false;

		if (isHovered && App->input->GetMouseButtonDown(1) == KEY_DOWN)
			isPressed = true;

		if (isPressed)
			rectTransform->SetPositionUsingAligment(float2(screenX, -screenY));

		if (isPressed && App->input->GetMouseButtonDown(1) == KEY_UP)
			isPressed = false;
	}*/
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
	value->AddFloat4("color", color);
	value->AddInt("FlipVertical", flipVertical);
	value->AddInt("FlipHorizontal", flipHorizontal);
	value->AddInt("isMasked", isMasked);
	value->AddInt("maskAmount", maskAmount);
	value->AddInt("isMaskHorizontal", isMaskHorizontal);
	
}

void ComponentImage::Load(JSON_value* value)
{
	Component::Load(value);
	unsigned uid = value->GetUint("textureUID");
	texture = (ResourceTexture*)App->resManager->Get(uid);
	color = value->GetFloat4("color");	
	flipVertical = value->GetInt("FlipVertical");
	flipHorizontal = value->GetInt("FlipHorizontal");
	isMasked = value->GetInt("isMasked");
	maskAmount = value->GetInt("maskAmount");
	isMaskHorizontal = value->GetInt("isMaskHorizontal");
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
