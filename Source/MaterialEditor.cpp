#include "Application.h"

#include "ModuleProgram.h"
#include "ModuleFileSystem.h"
#include "ModuleTextures.h"
#include "ModuleResourceManager.h"

#include "Resource.h"
#include "ResourceTexture.h"
#include "ResourceMaterial.h"

#include "MaterialEditor.h"
#include "imgui.h"

#define materialPopup "Material"

MaterialEditor::MaterialEditor()
{
}


MaterialEditor::~MaterialEditor()
{
}

void MaterialEditor::Draw()
{
	if (material == NULL)
	{
		return;
	}

	ImGui::Spacing();
	
	ImGui::ColorEdit3("Specular Color", (float*)&material->specularColor);
	ImGui::DragFloat("Roughness", &material->roughness, .01f, .001f, 1.f);
	
	ShaderSelector(currentShader);

	if (ImGui::CollapsingHeader("Diffuse"))
	{
		ImGui::PushID(&material->diffuseColor);
		ImGui::ColorEdit4("Color", (float*)&material->diffuseColor, ImGuiColorEditFlags_AlphaPreview);
		TextureSelector((unsigned)TextureType::DIFFUSE, currentDiffuse, 0);
		ImGui::Separator();
		ImGui::PopID();
	}
	if (ImGui::CollapsingHeader("Occlusion"))
	{
		ImGui::PushID(&material->textures[(unsigned)TextureType::OCCLUSION]);
		TextureSelector((unsigned)TextureType::OCCLUSION, currentOcclusion, 2);
		ImGui::Separator();
		ImGui::PopID();
	}
	if (ImGui::CollapsingHeader("Emissive"))
	{
		ImGui::ColorEdit3("Color", (float*)&material->emissiveColor);
		TextureSelector((unsigned)TextureType::EMISSIVE, currentEmissive, 3);
		ImGui::Separator();
	}
	if (ImGui::CollapsingHeader("Normal"))
	{
		TextureSelector((unsigned)TextureType::NORMAL, currentNormal, 4);
	}

	ImGui::SetCursorPosX(ImGui::GetWindowWidth()/2 - ImGui::CalcTextSize("Cancel Changes").x /2);
	if (ImGui::Button("Cancel Changes"))
	{
		material->Reset(*previous);
		SetCurrentTextures();
	}

}

void MaterialEditor::ShaderSelector(std::string & current_shader)
{
	if (shaders.size() == 0)
	{
		shaders = App->fsystem->GetFolderContent(VERTEXSHADERS, false);
	}
	if (material->shader != nullptr)
	{
		current_shader = material->shader->file;
	}
	if (ImGui::BeginCombo("Shader", current_shader.c_str()))
	{
		for (int n = 0; n < shaders.size(); n++)
		{
			bool is_selected = (current_shader == shaders[n]);
			if (ImGui::Selectable(shaders[n].c_str(), is_selected) && current_shader != shaders[n])
			{
				current_shader = shaders[n];
				material->shader = App->program->GetProgram(current_shader.c_str());
			}
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
}

void MaterialEditor::TextureSelector(unsigned i, std::string &current_texture, int id)
{
	ImGui::PushID(id);

	if (ImGui::BeginCombo("Texture", material->textures[i] != nullptr ? material->textures[i]->GetName() : None))
	{
		bool none_selected = (current_texture == None);
		if (ImGui::Selectable(None, none_selected))
		{
			current_texture = None;
			if (material->textures[i] != nullptr)
			{
				App->resManager->DeleteResource(material->textures[i]->GetUID());
				material->textures[i] = nullptr;
			}
		}
		if (none_selected)
			ImGui::SetItemDefaultFocus();
		for (int n = 0; n < textureFiles.size(); n++)
		{
			bool is_selected = (current_texture == textureFiles[n]);
			if (ImGui::Selectable(textureFiles[n].c_str(), is_selected) && !is_selected)
			{
				if(material->textures[i] != nullptr)
					App->resManager->DeleteResource(material->textures[i]->GetUID());

				current_texture = textureFiles[n];
				material->textures[i] = (ResourceTexture*)App->resManager->GetByName(current_texture.c_str(), TYPE::TEXTURE);
			}
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	if (material->textures[i] != nullptr)
	{
		ImGui::Image((ImTextureID)material->textures[i]->gpuID, { 200,200 }, { 0,1 }, { 1,0 });
	}
	ImGui::PopID();
}

void MaterialEditor::SetCurrentTextures()
{
	// Get shader
	if (material->shader != nullptr)
	{
		currentShader = material->shader->file;
	}
	else
	{
		currentShader = DEFAULTPROGRAM;
	}

	// Get material textures
	ResourceTexture* diffuse_texture = material->GetTexture(TextureType::DIFFUSE);
	ResourceTexture* specular_texture = material->GetTexture(TextureType::SPECULAR);
	ResourceTexture* occlusion_texture = material->GetTexture(TextureType::OCCLUSION);
	ResourceTexture* emissive_texture = material->GetTexture(TextureType::EMISSIVE);
	ResourceTexture* normal_texture = material->GetTexture(TextureType::NORMAL);

	// Set current textures strings
	if (diffuse_texture != nullptr)		
	{ 
		currentDiffuse = diffuse_texture->GetName(); 
	}
	else 
	{ 
		currentDiffuse = None; 
	}
	if (specular_texture != nullptr)	
	{ 
		currentSpecular = specular_texture->GetName();
	}
	else 
	{ 
		currentSpecular = None;
	}
	if (occlusion_texture != nullptr)	
	{ 
		currentOcclusion = occlusion_texture->GetName();
	}
	else 
	{ 
		currentOcclusion = None;
	}
	if (emissive_texture != nullptr)	
	{ 
		currentEmissive = emissive_texture->GetName();
	}
	else 
	{ 
		currentEmissive = None; 
	}
	if (normal_texture != nullptr)		
	{ 
		currentNormal = normal_texture->GetName();
	}
	else 
	{ 
		currentNormal = None; 
	}
}

void MaterialEditor::UpdateTexturesList()
{
	textureFiles.clear();
	textureFiles = App->resManager->GetResourceNamesList(TYPE::TEXTURE, true);
	SetCurrentTextures();
}

void MaterialEditor::NewMaterial()
{
	if (!ImGui::IsPopupOpen(materialPopup))
	{
		ImGui::OpenPopup(materialPopup);
	}
	if (ImGui::BeginPopupModal(materialPopup, NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Create new material:");
		ImGui::Separator();
		ImGui::InputText("NewName", newName, 64);

		if (strcmp(newName, newNamePrev) != 0)
		{
			if (Exists(newName))
			{
				newMatExists = true;
			}
			else
			{
				newMatExists = false;
			}
			strcpy(newNamePrev, newName);
		}

		if (newMatExists)
		{
			std::string nameTest = newName + std::string(" already exists!");
			ImGui::Text(nameTest.c_str());
		}

		if (ImGui::Button("Save", ImVec2(120, 0)) && !newMatExists)
		{
			ResourceMaterial* newMaterialCreated = (ResourceMaterial*)App->resManager->CreateNewResource(TYPE::MATERIAL);
			std::string newFile(MATERIALS);
			newFile += newName;
			newFile += MATERIALEXT;
			newMaterialCreated->SetFile(newFile.c_str());
			newMaterialCreated->SetName(newName);
			newMaterialCreated->Save();
			newMaterial = false;

			// Delete new material (It will be added to resources when the file on Assets is imported)
			App->resManager->DeleteResourceFromList(newMaterialCreated->GetUID());
			RELEASE(newMaterialCreated);
			strcpy(newName, "New Material");
			strcpy(newNamePrev, "");
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			newMaterial = false;
			strcpy(newName, "New Material");
			strcpy(newNamePrev, "");
		}
		ImGui::EndPopup();
	}
}

bool MaterialEditor::Exists(const std::string& material) const
{
	return App->fsystem->Exists((MATERIALS + material + MATERIALEXT).c_str());
}

void MaterialEditor::Save()
{
	if (previous != nullptr)
	{
		if (!material->Compare(*previous))
			material->Save();

		RELEASE(previous);
	}
}

void MaterialEditor::CleanUp()
{
	currentShader = None;
	currentDiffuse = None;
	currentSpecular = None;
	currentOcclusion = None;
	currentEmissive = None;

	textureFiles.clear();
	shaders.clear();
	open = false;

	if (previous != nullptr)
	{
		RELEASE(previous);
	}
}
