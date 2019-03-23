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
	mesh = (ResourceMesh*)App->resManager->Get(component.mesh->GetUID());
	material = (ResourceMaterial*)App->resManager->Get(component.material->GetUID());
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
		ImGui::Text("Num vertices : %d", mesh->meshVertices.size());
		ImGui::Text("Num triangles : %d", mesh->meshIndices.size() / 3);
		ImGui::Spacing();

		ImGui::Text("Material");
		if (ImGui::BeginCombo("", material->name.c_str()))
		{
			if (guiMaterials.empty())
			{
				guiMaterials = App->fsystem->GetFolderContent(IMPORTED_MATERIALS, false);
			}
			for (int n = 0; n < guiMaterials.size(); n++)
			{
				bool is_selected = (material->name == guiMaterials[n]);
				if (ImGui::Selectable(guiMaterials[n].c_str(), is_selected) && material->name != guiMaterials[n])
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
		}
		else
		{
			if (ImGui::Button("Show"))
			{
				App->editor->materialEditor->open = true;
				App->editor->materialEditor->material = material;
				App->editor->materialEditor->previous = new ResourceMaterial(*material);
				App->editor->materialEditor->SetCurrentTextures();
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
	value->AddUint("meshUID", mesh->GetUID());
	value->AddString("materialFile", material->name.c_str());
}

void ComponentRenderer::Load(JSON_value* value)
{
	Component::Load(value);

	unsigned uid = value->GetUint("meshUID");
	ResourceMesh* m = (ResourceMesh*)App->resManager->Get(uid); //Look for loaded meshes

	if (m != nullptr)
	{
		mesh = m;
	}
	else //Case mesh not loaded
	{
		ResourceMesh* res = (ResourceMesh*)App->resManager->CreateNewResource(TYPE::MESH, uid);
		res->SetExportedFile(std::to_string(uid).c_str());
		m = (ResourceMesh*)App->resManager->Get(uid); //Look for loaded meshes
		if (m != nullptr)
			m = res;
	}
	UpdateGameObject();

	const char* materialFile = value->GetString("materialFile");
	SetMaterial(materialFile);
}

void ComponentRenderer::SetMaterial(const char * materialfile)
{
	if (materialfile == nullptr)
	{
		materialfile = App->resManager->Get(DEFAULTMAT)->GetExportedFile();
	}

	if (material == nullptr || material->name != materialfile)
	{
		if (material != nullptr)
		{
			App->resManager->DeleteResource(material->GetUID());
		}

		material = (ResourceMaterial*)App->resManager->Get(materialfile);
	}
	return;
}

void ComponentRenderer::UpdateMesh(const char * data, unsigned uid)
{
	//mesh->SetMesh(data);
	UpdateGameObject();
}

void ComponentRenderer::UpdateGameObject()
{
	if (gameobject != nullptr)
	{
		gameobject->UpdateBBox();
		App->scene->AddToSpacePartition(gameobject);
	}
}
