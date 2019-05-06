#include "Application.h"

#include "ModuleResourceManager.h"
#include "ModuleFileSystem.h"
#include "ModuleEditor.h"
#include "ModuleScene.h"

#include "GameObject.h"
#include "ComponentRenderer.h"

#include "Resource.h"
#include "ResourceMesh.h"
#include "ResourceMaterial.h"

#include "myQuadTree.h"
#include "MaterialEditor.h"
#include "JSON.h"

#include "imgui.h"
#include "Math/float4x4.h"

ComponentRenderer::ComponentRenderer(GameObject* gameobject) : Component(gameobject, ComponentType::Renderer)
{
	SetMaterial(DEFAULTMAT);
	gameobject->isVolumetric = true;
}

ComponentRenderer::ComponentRenderer(const ComponentRenderer& component) : Component(component)
{
	if(component.mesh != nullptr)
		mesh = (ResourceMesh*)App->resManager->Get(component.mesh->GetUID());

	if (component.material != nullptr)
		material = (ResourceMaterial*)App->resManager->Get(component.material->GetUID());

	castShadows = component.castShadows;
}

ComponentRenderer::~ComponentRenderer()
{
	//Resource Manager Deallocates resources (materials, meshes)
	if (mesh != nullptr)
	{
		App->resManager->DeleteResource(mesh->GetUID());
		mesh = nullptr;
	}

	if (material != nullptr)
	{
		App->resManager->DeleteResource(material->GetUID());
		material = nullptr;
	}
}

Component * ComponentRenderer::Clone() const
{
	return new ComponentRenderer(*this);
}

void ComponentRenderer::DrawProperties()
{
	ImGui::PushID(this);
	if (ImGui::CollapsingHeader("Mesh Renderer", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool removed = Component::DrawComponentState();
		if (removed)
		{
			ImGui::PopID();
			return;
		}

		// Mesh selector
		ImGui::Text("Mesh");
		ImGui::PushID("Mesh Combo");
		if (ImGui::BeginCombo("", mesh != nullptr ? mesh->name.c_str() : ""))
		{
			if (guiMeshes.empty())
			{
				guiMeshes = App->resManager->GetMeshesNamesList(true);
			}
			for (int n = 0; n < guiMeshes.size(); n++)
			{
				bool is_selected = (mesh != nullptr ? mesh->name == guiMeshes[n] : false);
				if (ImGui::Selectable(guiMeshes[n].c_str(), is_selected))
				{
					if(mesh == nullptr || mesh->name != guiMeshes[n])
						SetMesh(guiMeshes[n].c_str());
				}
				if (is_selected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		else
		{
			guiMeshes.clear();
		}
		ImGui::PopID();

		if (mesh == nullptr)
		{
			ImGui::Text("No mesh selected.");
		}
		else
		{
			ImGui::Text("Num vertices : %d", mesh->meshVertices.size());
			ImGui::Text("Num triangles : %d", mesh->meshIndices.size() / 3);
		}
		ImGui::Spacing();
		ImGui::Checkbox("Cast shadows", &castShadows);
		ImGui::Checkbox("Use Alpha", &useAlpha);

		// Material selector
		ImGui::Text("Material");
		ImGui::PushID("Material Combo");
		if (ImGui::BeginCombo("", material->GetExportedFile()))
		{
			if (guiMaterials.empty())
			{
				guiMaterials = App->resManager->GetResourceNamesList(TYPE::MATERIAL, true);
			}
			for (int n = 0; n < guiMaterials.size(); n++)
			{
				bool is_selected = (material->GetExportedFile() == guiMaterials[n]);
				if (ImGui::Selectable(guiMaterials[n].c_str(), is_selected) && material->GetExportedFile() != guiMaterials[n])
				{
					SetMaterial(guiMaterials[n].c_str());

					if (App->editor->materialEditor->open)
					{
						App->editor->materialEditor->material = material;
						App->editor->materialEditor->previous = new ResourceMaterial(*material);
						App->editor->materialEditor->SetCurrentTextures();
					}
				}
				if (is_selected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		else
		{
			guiMaterials.clear();
		}
		ImGui::PopID();

		ImGui::SameLine();
		if (App->editor->materialEditor->open)
		{
			if (ImGui::Button("Hide"))
			{
				App->editor->materialEditor->open = false;

				if (!App->editor->materialEditor->material->Compare(*App->editor->materialEditor->previous))
				{
					App->editor->materialEditor->material->Save();
				}
				App->editor->materialEditor->CleanUp();
			}

			if (ImGui::Button("Refresh Material"))
			{
				App->editor->materialEditor->UpdateTexturesList();
			}
		}
		else
		{
			if (ImGui::Button("Show"))
			{
				App->editor->materialEditor->open = true;
				App->editor->materialEditor->material = material;
				App->editor->materialEditor->previous = new ResourceMaterial(*material);
				App->editor->materialEditor->SetCurrentTextures();

				// Update texture list
				App->editor->materialEditor->UpdateTexturesList();
			}
		}

		if (App->editor->materialEditor->open)
		{
			App->editor->materialEditor->Draw();
		}

		ImGui::Separator();
	}

	ImGui::PopID();
}

bool ComponentRenderer::CleanUp()
{
	if (mesh != nullptr)
	{
		App->resManager->DeleteResource(mesh->GetUID());
		App->scene->DeleteFromSpacePartition(gameobject);
		mesh = nullptr;
	}

	if (material != nullptr)
	{
		App->resManager->DeleteResource(material->GetUID());
		material = nullptr;
	}
	return true;
}

void ComponentRenderer::Save(JSON_value* value) const
{
	Component::Save(value);
	value->AddUint("meshUID", (mesh != nullptr) ? mesh->GetUID() : 0u);
	value->AddString("materialFile", (material != nullptr) ? material->GetExportedFile() : DEFAULTMAT);
	value->AddInt("castShadows", castShadows);
	value->AddInt("useAlpha", useAlpha);
}

void ComponentRenderer::Load(JSON_value* value)
{
	Component::Load(value);

	unsigned uid = value->GetUint("meshUID");
	mesh = (ResourceMesh*)App->resManager->Get(uid); //Look for loaded meshes
	UpdateGameObject();

	const char* materialFile = value->GetString("materialFile");
	SetMaterial(materialFile);

	castShadows = value->GetInt("castShadows");
	useAlpha = value->GetInt("useAlpha");
}

void ComponentRenderer::SetMaterial(const char* materialfile)
{
	// Delete previous material
	if (material != nullptr)
	{
		App->resManager->DeleteResource(material->GetUID());
	}

	if (materialfile == nullptr)
	{
		material = (ResourceMaterial*)App->resManager->Get(DEFAULTMAT,TYPE::MATERIAL);
		return;
	}
	else
	{
		material = (ResourceMaterial*)App->resManager->Get(materialfile, TYPE::MATERIAL);

		// Material can't be found
		if(material == nullptr)
			material = (ResourceMaterial*)App->resManager->Get(DEFAULTMAT, TYPE::MATERIAL);
	}
	return;
}

void ComponentRenderer::SetMesh(const char* meshfile)
{
	// Delete previous mesh
	if (mesh != nullptr)
		App->resManager->DeleteResource(mesh->GetUID());

	if (meshfile != nullptr)
		mesh = (ResourceMesh*)App->resManager->GetMeshByName(meshfile);

	LinkBones();
	UpdateGameObject();
	return;
}

void ComponentRenderer::UpdateGameObject()
{
	if (gameobject != nullptr)
	{
		gameobject->UpdateBBox();
		App->scene->AddToSpacePartition(gameobject);
	}
}

void ComponentRenderer::LinkBones() const
{
	mesh->LinkBones(gameobject);
}
