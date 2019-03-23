#include "ComponentImage.h"

#include "Globals.h"
#include "Application.h"
#include "ModuleResourceManager.h"
#include "ModuleTextures.h"
#include "ModuleFileSystem.h"
#include "ModuleUI.h"

#include "ResourceTexture.h"

#include "JSON.h"
#include "imgui.h"

#define None "None Selected"

ComponentImage::ComponentImage() : Component(nullptr, ComponentType::Image)
{
	//Refact
	if (textureFiles.size() == 0)
	{
		textureFiles = App->fsystem->GetFolderContent(TEXTURES, false);
	}
	App->ui->images.push_back(this);
}

ComponentImage::ComponentImage(GameObject* gameobject) : Component(gameobject, ComponentType::Image)
{
	if (textureFiles.size() == 0)
	{
		textureFiles = App->fsystem->GetFolderContent(TEXTURES, false);
	}
	App->ui->images.push_back(this);
}

ComponentImage::ComponentImage(const ComponentImage &copy) : Component(copy)
{
	color = copy.color;
	textureName = copy.textureName;
	App->ui->images.push_back(this);
}

ComponentImage::~ComponentImage()
{
	App->ui->images.remove(this);
	// ResManager refactored:
	//App->resManager->DeleteTexture(textureName);
	unsigned imageUID = App->resManager->FindByExportedFile(textureName.c_str());
	App->resManager->DeleteResource(imageUID);
	texture = nullptr;
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
		if (ImGui::BeginCombo("Texture", textureName.c_str()))
		{
			bool none_selected = (textureName == None);
			if (ImGui::Selectable(None, none_selected))
			{
				if (texture != nullptr)
				{
					// ResManager refactored:
					//App->resManager->DeleteTexture(textureName);
					unsigned imageUID = App->resManager->FindByExportedFile(textureName.c_str());
					App->resManager->DeleteResource(imageUID);
					texture = nullptr;
				}
				textureName = None;
			}
			if (none_selected)
				ImGui::SetItemDefaultFocus();
			for (int n = 0; n < textureFiles.size(); n++)
			{
				bool is_selected = (textureName == textureFiles[n]);
				if (ImGui::Selectable(textureFiles[n].c_str(), is_selected) && !is_selected)
				{
					App->resManager->DeleteResource(App->resManager->FindByExportedFile(textureName.c_str()));
					textureName = textureFiles[n].c_str();
					// ResManager refactored:
					//texture = App->textures->GetTexture(textureName.c_str());
					unsigned imageUID = App->resManager->FindByExportedFile(textureName.c_str());
					texture = (ResourceTexture*)App->resManager->Get(imageUID);
				}
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		if (texture != nullptr)
		{
			ImGui::Image((ImTextureID)texture->gpuID, { 200,200 }, { 0,1 }, { 1,0 });
		}
		
		//color
		ImGui::ColorEdit4("Image color", (float*)&color);

		ImGui::Separator();
	}
}

void ComponentImage::Save(JSON_value *value)const
{
	Component::Save(value);
	value->AddString("textureName", textureName.c_str());
	value->AddFloat4("color", color);
}

void ComponentImage::Load(JSON_value* value)
{
	Component::Load(value);
	textureName = value->GetString("textureName");
	color = value->GetFloat4("color");	
	if (textureName != "None Selected")
	{
		// ResManager refactored:
		//texture = App->textures->GetTexture(textureName.c_str());
		texture = (ResourceTexture*)App->resManager->Get(textureName.c_str());
	}
}