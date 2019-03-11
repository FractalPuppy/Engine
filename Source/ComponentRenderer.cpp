#include "Application.h"

#include "ModuleResourceManager.h"
#include "ModuleFileSystem.h"
#include "ModuleEditor.h"
#include "ModuleScene.h"

#include "GameObject.h"
#include "ComponentRenderer.h"

#include "myQuadTree.h"
#include "MaterialEditor.h"
#include "Material.h"
#include "Mesh.h"
#include "JSON.h"

#include "imgui.h"
#include "Math/float4x4.h"

ComponentRenderer::ComponentRenderer(GameObject * gameobject) : Component(gameobject, ComponentType::Renderer)
{
	mesh = new Mesh();
	SetMaterial(DEFAULTMAT);
	gameobject->isVolumetric = true;
}

ComponentRenderer::ComponentRenderer(const ComponentRenderer & component) : Component(component)
{
	mesh = component.mesh;
	material = component.material;

	App->resManager->AddMesh(mesh);
	App->resManager->AddMaterial(material);
}

ComponentRenderer::~ComponentRenderer()
{
	material = nullptr; //Resource Manager Deallocates resources (materials, meshes)
	mesh = nullptr;
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
				guiMaterials = App->fsystem->ListFiles(MATERIALS, false);
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
						App->editor->materialEditor->previous = new Material(*material);
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
			}
		}
		else
		{
			if (ImGui::Button("Show"))
			{
				App->editor->materialEditor->open = true;
				App->editor->materialEditor->material = material;
				App->editor->materialEditor->previous = new Material(*material);
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
	if (material != nullptr)
	{
		App->resManager->DeleteMaterial(material->name);
	}
	if (mesh != nullptr)
	{
		App->resManager->DeleteMesh(mesh->UID);
	}
	return true;
}

void ComponentRenderer::Save(JSON_value* value) const
{
	Component::Save(value);
	value->AddUint("meshUID", mesh->UID);
	value->AddString("materialFile", material->name.c_str());
}

void ComponentRenderer::Load(JSON_value* value)
{
	Component::Load(value);
	unsigned uid = value->GetUint("meshUID");
	App->resManager->DeleteMesh(mesh->UID); //Delete existing old mesh
	Mesh *m = App->resManager->GetMesh(uid); //Look for loaded meshes
	if (m != nullptr)
	{
		mesh = m;
	}
	else //Case mesh not loaded
	{
		char *data = nullptr;
		App->fsystem->Load((MESHES + std::to_string(uid) + MESHEXTENSION).c_str(), &data);
		mesh->SetMesh(data, uid); //Deallocates data
	}
	App->resManager->AddMesh(mesh);
	UpdateGameObject();

	const char* materialFile = value->GetString("materialFile");
	SetMaterial(materialFile);
}

void ComponentRenderer::SetMaterial(const char * materialfile)
{
	if (materialfile == nullptr)
	{
		materialfile = DEFAULTMAT;
	}

	if (material == nullptr || material->name != materialfile)
	{
		if (material != nullptr)
		{
			App->resManager->DeleteMaterial(material->name);
		}

		Material *mat = App->resManager->GetMaterial(materialfile);
		if (mat == nullptr)
		{
			material = new Material();
			material->Load(materialfile);
		}
		else
		{
			material = mat;
		}
		App->resManager->AddMaterial(material);
	}
	return;
}

void ComponentRenderer::UpdateMesh(const char * data, unsigned uid)
{
	mesh->SetMesh(data, uid);
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