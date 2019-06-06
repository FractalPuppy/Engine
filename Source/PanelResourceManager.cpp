#include "PanelResourceManager.h"

#include "Application.h"
#include "ModuleResourceManager.h"
#include "ModuleProgram.h"

#include "Resource.h"
#include "ResourceTexture.h"
#include "ResourceModel.h"
#include "ResourceMesh.h"
#include "ResourceStateMachine.h"
#include "ResourceMaterial.h"
#include "ResourceSkybox.h"
#include "ResourceAnimation.h."
#include "ResourceScene.h"

#include "imgui.h"
#include <algorithm>
#define MAX_FILE 128
#define resourcePopup "Resource"

#pragma region sorting functions
bool sortByUIDAscending(const Resource* a, const Resource* b) { return a->GetUID() < b->GetUID(); };
bool sortByUIDDescending(const Resource* a, const Resource* b) { return a->GetUID() > b->GetUID(); };
bool sortByNameAscending(const Resource* a, const Resource* b)
{
	std::string stringA(a->GetName());
	std::string stringB(b->GetName());
	return stringA < stringB;
};

bool sortByNameDescending(const Resource* a, const Resource* b)
{
	std::string stringA(a->GetName());
	std::string stringB(b->GetName());
	return stringA > stringB;
};
bool sortByFileAscending(const Resource* a, const Resource* b)
{
	std::string stringA(a->GetFile());
	std::string stringB(b->GetFile());
	return stringA < stringB;
};

bool sortByFileDescending(const Resource* a, const Resource* b)
{
	std::string stringA(a->GetFile());
	std::string stringB(b->GetFile());
	return stringA > stringB;
};

bool sortByExportedFileAscending(const Resource* a, const Resource* b)
{
	std::string stringA(a->GetExportedFile());
	std::string stringB(b->GetExportedFile());
	return stringA < stringB;
};

bool sortByExportedFileDescending(const Resource* a, const Resource* b)
{
	std::string stringA(a->GetExportedFile());
	std::string stringB(b->GetExportedFile());
	return stringA > stringB;
};

bool sortByReferenceAscending(const Resource* a, const Resource* b) { return a->GetReferences() < b->GetReferences(); };
bool sortByReferenceDescending(const Resource* a, const Resource* b) { return a->GetReferences() > b->GetReferences(); };
bool sortByTypeAscending(const Resource* a, const Resource* b) { return a->GetType() < b->GetType(); };
bool sortByTypeDescending(const Resource* a, const Resource* b) { return a->GetType() > b->GetType(); };
#pragma endregion

PanelResourceManager::PanelResourceManager()
{
	enabled = false;
}

PanelResourceManager::~PanelResourceManager()
{
}

void PanelResourceManager::Draw()
{
	if (!ImGui::Begin("Resource Manager", &enabled, ImGuiWindowFlags_MenuBar))
	{
		ImGui::End();
		return;
	}
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Resources"))
		{
			if (ImGui::MenuItem("Update Resources List"))
			{
				UpdateResourcesList();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Options"))
		{
			if (ImGui::MenuItem("Delete Unused Metas"))
			{
				App->resManager->CleanUnusedMetaFiles();
			}
			if (ImGui::MenuItem("Delete Unused Exported Files"))
			{
				App->resManager->CleanUnusedExportedFiles();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	if(auxResource == nullptr)
		UpdateResourcesList();

	ImGui::Columns(7);
	// Table references: UID | File | Exported File | References | Type |
	ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor::HSV(0.0f, 1.0f, 1.0f));
	if (ImGui::Selectable("UID"))			{ if (sortList == SORTING::UID) descending = !descending; sortList = SORTING::UID; }				ImGui::SameLine(); ImGui::NextColumn();
	if (ImGui::Selectable("Name"))			{ if (sortList == SORTING::NAME) descending = !descending; sortList = SORTING::NAME; }				ImGui::SameLine(); ImGui::NextColumn();
	if (ImGui::Selectable("File"))			{ if (sortList == SORTING::FILE) descending = !descending; sortList = SORTING::FILE;}				ImGui::SameLine(); ImGui::NextColumn();
	if (ImGui::Selectable("Exported File"))	{ if (sortList == SORTING::EXPORTED) descending = !descending; sortList = SORTING::EXPORTED;}		ImGui::SameLine(); ImGui::NextColumn();
	if (ImGui::Selectable("References"))	{ if (sortList == SORTING::REFERENCES) descending = !descending; sortList = SORTING::REFERENCES;}	ImGui::SameLine(); ImGui::NextColumn();
	if (ImGui::Selectable("Type"))			{ if (sortList == SORTING::TYPE) descending = !descending; sortList = SORTING::TYPE;}				ImGui::SameLine(); ImGui::NextColumn();
	ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "");																						ImGui::NextColumn(); ImGui::Separator();
	ImGui::PopStyleColor(1);

	for each (auto& resource in resourcesList)
	{
		unsigned uid = resource->GetUID();
		ImGui::PushID(uid);

		// If the resource is used by the engine show the text disabled
		bool engineResource = resource->IsUsedByEngine();

		// UID
		!engineResource? ImGui::Text("%u", uid) : ImGui::TextDisabled("%u", uid);
		ImGui::NextColumn();

		// Name
		!engineResource ? ImGui::Text(resource->GetName()) : ImGui::TextDisabled(resource->GetName());
		ImGui::NextColumn();

		// File
		!engineResource ? ImGui::Text(resource->GetFile()) : ImGui::TextDisabled(resource->GetFile());
		ImGui::NextColumn();

		// Exported File
		!engineResource ? ImGui::Text(resource->GetExportedFile()) : ImGui::TextDisabled(resource->GetExportedFile());
		ImGui::NextColumn();

		// Loaded
		!engineResource ? ImGui::Text("%u", resource->GetReferences()) : ImGui::TextDisabled("%u", resource->GetReferences());
		ImGui::NextColumn();

		// Type
		switch (resource->GetType())
		{
		case TYPE::TEXTURE:		!engineResource ? ImGui::Text("Texture")		: ImGui::TextDisabled("Texture");		break;
		case TYPE::MODEL:		!engineResource ? ImGui::Text("Model")			: ImGui::TextDisabled("Model");			break;
		case TYPE::MESH:		!engineResource ? ImGui::Text("Mesh")			: ImGui::TextDisabled("Mesh");			break;
		case TYPE::AUDIO:		!engineResource ? ImGui::Text("Audio")			: ImGui::TextDisabled("Audio");			break;
		case TYPE::SCENE:		!engineResource ? ImGui::Text("Scene")			: ImGui::TextDisabled("Scene");			break;
		case TYPE::ANIMATION:	!engineResource ? ImGui::Text("Animation")		: ImGui::TextDisabled("Animation");		break;
		case TYPE::MATERIAL:	!engineResource ? ImGui::Text("Material")		: ImGui::TextDisabled("Material");		break;
		case TYPE::SKYBOX:		!engineResource ? ImGui::Text("Skybox")			: ImGui::TextDisabled("Skybox");		break;
		case TYPE::STATEMACHINE:!engineResource ? ImGui::Text("StateMachine")	: ImGui::TextDisabled("StateMachine");	break;
		default:
		case TYPE::UNKNOWN:		!engineResource ? ImGui::Text("Unknown")		: ImGui::TextDisabled("Unknown");		break;
		}
		ImGui::NextColumn();
		// View button
		if (ImGui::Button("View"))
		{
			openResourceWindow = true;
			previous = resource;
		}
		ImGui::SameLine();
		// Edit button
		if (ImGui::Button("Edit"))
		{
			openEditor = true;
			previous = resource;
			auxUID = resource->GetUID();
		}
		ImGui::NextColumn();
		ImGui::PopID();
	}
	OpenResourceEditor();
	if (openResourceWindow)
	{
		switch (previous->GetType())
		{
		case TYPE::TEXTURE:		DrawResourceTexture();	break;
		case TYPE::MODEL:		DrawResourceModel();	break;
		case TYPE::MESH:		DrawResourceMesh();		break;
		/*case TYPE::AUDIO:								break;*/
		case TYPE::SCENE:		DrawResourceScene();	break;
		case TYPE::ANIMATION:	DrawResourceAnimation();break;
		case TYPE::MATERIAL:	DrawResourceMaterial(); break;
		case TYPE::SKYBOX:		DrawResourceSkybox();	break;
		case TYPE::STATEMACHINE: DrawResourceSM();		break;
		}
	}
	ImGui::End();
}

void PanelResourceManager::UpdateResourcesList()
{
	resourcesList = App->resManager->GetResourcesList();

	switch (sortList)
	{
	default:
	case SORTING::NONE:
		break;
	case SORTING::UID:
		if(!descending) std::sort(resourcesList.begin(), resourcesList.end(), sortByUIDAscending);
		else 			std::sort(resourcesList.begin(), resourcesList.end(), sortByUIDDescending);
		break;
	case SORTING::NAME:
		if (!descending) std::sort(resourcesList.begin(), resourcesList.end(), sortByNameAscending);
		else			 std::sort(resourcesList.begin(), resourcesList.end(), sortByNameDescending);
		break;
	case SORTING::FILE:
		if (!descending) std::sort(resourcesList.begin(), resourcesList.end(), sortByFileAscending);
		else			 std::sort(resourcesList.begin(), resourcesList.end(), sortByFileDescending);
		break;
	case SORTING::EXPORTED:
		if (!descending) std::sort(resourcesList.begin(), resourcesList.end(), sortByExportedFileAscending);
		else			 std::sort(resourcesList.begin(), resourcesList.end(), sortByExportedFileDescending);
		break;
	case SORTING::REFERENCES:
		if (!descending) std::sort(resourcesList.begin(), resourcesList.end(), sortByReferenceAscending);
		else			 std::sort(resourcesList.begin(), resourcesList.end(), sortByReferenceDescending);
		break;
	case SORTING::TYPE:
		if (!descending) std::sort(resourcesList.begin(), resourcesList.end(), sortByTypeAscending);
		else			 std::sort(resourcesList.begin(), resourcesList.end(), sortByTypeDescending);
		break;
	}
}

void PanelResourceManager::OpenResourceEditor()
{
	if (openEditor)
	{
		if (!ImGui::IsPopupOpen(resourcePopup))
		{
			ImGui::OpenPopup(resourcePopup);
			auxResource = new Resource(*previous);
			// Use a fake reference to avoid deleting the texture from memory 
			auxReferences = auxResource->GetReferences();
		}

		ImGui::SetNextWindowSizeConstraints(ImVec2(200, 200), ImVec2(500, 500));
		if (ImGui::BeginPopupModal(resourcePopup, NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			// UID
			ImGui::Text("UID: %u", auxUID);

			// Name
			char name[64] = "";
			strcpy(name, auxResource->GetName());
			ImGui::InputText("Name", name, MAX_FILE);
			auxResource->SetName(name);

			// File
			char file[64] = "";
			strcpy(file, auxResource->GetFile());
			ImGui::InputText("File", file, MAX_FILE);
			auxResource->SetFile(file);

			// Exported File
			char exportedFile[64] = "";
			strcpy(exportedFile, auxResource->GetExportedFile());
			ImGui::InputText("Exported File", exportedFile, MAX_FILE);
			auxResource->SetExportedFile(exportedFile);

			// Loaded
			if (ImGui::InputInt("References", &auxReferences))
			{
				if (auxReferences < 0)
					auxReferences = 0;
			}

			// Used by engine
			bool used = auxResource->IsUsedByEngine();
			if (ImGui::Checkbox("Used by engine", &used))
			{
				auxResource->SetUsedByEngine(used);
			}

			// Type
			const char* types[] = { "Texture", "Model", "Mesh", "Audio", "Scene", "Animation", "Material", "Skybox", "State Machine", "Unknown" };
			int type = (int)auxResource->GetType();
			if (ImGui::BeginCombo("Type", types[type]))
			{
				for (int n = 0; n < (int)TYPE::UNKNOWN; n++)
				{
					bool is_selected = (type == n);
					if (ImGui::Selectable(types[n], is_selected) && type != n)
					{
						auxResource->SetType((TYPE)n);
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			if (ImGui::Button("Apply"))
			{
				auxResource->SetReferences(auxReferences);
				previous->Resource::Copy(*auxResource);
				CleanUp();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				CleanUp();
			}
			ImGui::EndPopup();
		}
	}
}

void PanelResourceManager::DrawResourceTexture()
{
	if (!ImGui::Begin("Texture Manager"))
	{
		ImGui::End();
		return;
	}
	ResourceTexture& texture = *(ResourceTexture*)previous;
	std::string exportedFile(texture.GetExportedFile());
	ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), (exportedFile +":").c_str());
	ImGui::Columns(2);
	ImGui::Text("Width: %u", texture.width);
	ImGui::Text("Height: %u", texture.height);
	ImGui::Text("Depth: %u", texture.depth);
	ImGui::Text("Mips: %u", texture.mips);
	ImGui::Text("Bytes: %u", texture.bytes);
	ImGui::Text("GPU ID: %u", texture.gpuID);
	ImGui::Text("Format: %u", texture.format);
	ImGui::NextColumn();
	// Texture Type
	const char* imageTypes[] = { "Texture", "Cubemap" };
	int type = (int)texture.GetImageType();
	if (ImGui::BeginCombo("Image type", imageTypes[type]))
	{
		for (int n = 0; n < (int)IMAGE_TYPE::CUBEMAP + 1; n++)
		{
			bool is_selected = (type == n);
			if (ImGui::Selectable(imageTypes[n], is_selected) && type != n)
			{
				texture.SetImageType((IMAGE_TYPE)n);
			}
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	ImGui::Image((ImTextureID)texture.gpuID, ImVec2(160.0f, 160.0f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
	
	ImGui::End();
}

void PanelResourceManager::DrawResourceModel()
{
	if (!ImGui::Begin("Model Manager"))
	{
		ImGui::End();
		return;
	}
	ResourceModel& model = *(ResourceModel*)previous;
	std::string exportedFile(model.GetExportedFile());
	ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), (exportedFile + ":").c_str());

	for each(auto& mesh in model.meshList)
	{
		ImGui::Columns(2);
		std::string exportedFile(mesh->GetExportedFile());
		ImGui::Text((exportedFile + ":").c_str());
		ImGui::Columns(2);
		ImGui::Text("VAO: %u", mesh->GetVAO());
		ImGui::Text("VBO: %u", mesh->GetVBO());
		ImGui::Text("EBO: %u", mesh->GetEBO());
		ImGui::Text("Number of Triangles: %u", mesh->meshVertices.size());
		ImGui::Text("Number of Vertices: %u", mesh->meshIndices.size() / 2);

		ImGui::NextColumn();
		// TODO: [Resource Manager] Add preview of the mesh

		ImGui::NextColumn();
		ImGui::Separator();
	}


	ImGui::End();
}

void PanelResourceManager::DrawResourceMesh()
{
	if (!ImGui::Begin("Mesh Manager"))
	{
		ImGui::End();
		return;
	}
	ResourceMesh& mesh = *(ResourceMesh*)previous;
	std::string exportedFile(mesh.GetExportedFile());
	ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), (exportedFile + ":").c_str());
	ImGui::Columns(2);
	ImGui::Text("Name: %s", mesh.GetName());
	ImGui::Text("VAO: %u", mesh.GetVAO());
	ImGui::Text("VBO: %u", mesh.GetVBO());
	ImGui::Text("EBO: %u", mesh.GetEBO());
	ImGui::Text("Number of Triangles: %u", mesh.meshVertices.size());
	ImGui::Text("Number of Vertices: %u", mesh.meshIndices.size() / 2);
	ImGui::NextColumn();
	// TODO: [Resource Manager] Add preview of the mesh

	ImGui::End();
}

void PanelResourceManager::DrawResourceScene()
{
	if (!ImGui::Begin("Scene Manager"))
	{
		ImGui::End();
		return;
	}
	ResourceScene& scene = *(ResourceScene*)previous;
	std::string exportedFile(scene.GetExportedFile());
	ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), (exportedFile + ":").c_str());
	ImGui::Columns(2);
	ImGui::Text("Name: %s", scene.GetName());

	ImGui::NextColumn();
	// TODO: [Resource Manager] Add preview of the scene
	ImGui::End();
}

void PanelResourceManager::DrawResourceAnimation()
{
	if (!ImGui::Begin("Animation Manager"))
	{
		ImGui::End();
		return;
	}
	ResourceAnimation& animation = *(ResourceAnimation*)previous;
	std::string exportedFile(animation.GetExportedFile());
	ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), (exportedFile + ":").c_str());
	ImGui::Columns(2);
	ImGui::Text("Name: %s", animation.GetName());
	ImGui::Text("Channels: %u", animation.GetNumberChannels());
	ImGui::Text("FPS: %u", animation.GetFPS());
	ImGui::Text("Total frames: %u", animation.GetNumberFrames());

	ImGui::NextColumn();
	// TODO: [Resource Manager] Add preview of the anim
	ImGui::End();
}


void PanelResourceManager::DrawResourceMaterial()
{
	if (!ImGui::Begin("Material Manager"))
	{
		ImGui::End();
		return;
	}
	ResourceMaterial& material = *(ResourceMaterial*)previous;
	std::string exportedFile(material.GetExportedFile());
	ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), (exportedFile + ":").c_str());
	ImGui::Columns(2);

	ImGui::Text("Name: %s", material.GetExportedFile());
	if(material.shader != nullptr)
		ImGui::Text("Shader: %s", material.shader->file.c_str());

	// Colors
	math::float4 diffuseColor = material.diffuseColor;
	ImGui::ColorEdit4("Diffuse Color", &(float&)diffuseColor, ImGuiColorEditFlags_NoInputs);
	math::float3 specularColor = material.specularColor;
	ImGui::ColorEdit3("Specular Color", &(float&)specularColor, ImGuiColorEditFlags_NoInputs);
	math::float3 emissiveColor = material.emissiveColor;
	ImGui::ColorEdit3("Emissive Color", &(float&)emissiveColor, ImGuiColorEditFlags_NoInputs);

	ImGui::Text("Roughness: %f", material.roughness);
	ImGui::Text("Metallic: %f", material.metallic);
	ImGui::NextColumn();

	// Textures
	unsigned i = 0;
	for each(auto& texture in material.textures)
	{
		i++;
		ImGui::Text("Texture %u:", i); ImGui::SameLine();
		if (texture != nullptr)
		{
			ImGui::Text(texture->GetExportedFile());
			ImGui::Image((ImTextureID)texture->gpuID, ImVec2(100.0f, 100.0f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
		}
		else
		{
			ImGui::Text("NULL");
			ImGui::Image(0, ImVec2(100.0f, 100.0f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
		}
	}
	ImGui::NextColumn();
	// TODO: [Resource Manager] Add preview of the material on a sphere

	ImGui::End();
}

void PanelResourceManager::DrawResourceSkybox()
{
	if (!ImGui::Begin("Skybox Manager"))
	{
		ImGui::End();
		return;
	}
	ResourceSkybox& skybox = *(ResourceSkybox*)previous;
	std::string exportedFile(skybox.GetExportedFile());
	ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), (exportedFile + ":").c_str());
	ImGui::Columns(2);

	// Skybox variables
	if (skybox.shader != nullptr)
		ImGui::Text("Shader: %s", skybox.shader->file.c_str());

	ImGui::Text("Cubemap: %u", skybox.GetCubemap());
	ImGui::Text("VAO: %u", skybox.GetVAO());
	ImGui::Text("VBO: %u", skybox.GetVBO());
	ImGui::NextColumn();
	// TODO: [Resource Manager] Add preview of the skybox on a sphere

	ImGui::End();
}

void PanelResourceManager::DrawResourceSM()
{
	if (!ImGui::Begin("StateMachine Manager"))
	{
		ImGui::End();
		return;
	}
	ResourceStateMachine& stateMachine = *(ResourceStateMachine*)previous;
	std::string exportedFile(stateMachine.GetExportedFile());
	ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), (exportedFile + ":").c_str());
	ImGui::Columns(2);

	// Skybox variables
	

	ImGui::NextColumn();
	// TODO: [Resource Manager] Add preview of the skybox on a sphere

	ImGui::End();
}

void PanelResourceManager::CleanUp()
{
	if(!openResourceWindow)
		previous = nullptr;

	if (auxResource != nullptr)
		RELEASE(auxResource);

	openEditor = false;
}