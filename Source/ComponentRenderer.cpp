#include "Application.h"
#include "GL/glew.h"

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
#include "HashString.h"

#include <stack>
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
	useAlpha = component.useAlpha;
	highlighted = component.highlighted;
	highlightColor = component.highlightColor;
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
		if (ImGui::BeginCombo("", mesh != nullptr ? mesh->GetName() : "None selected"))
		{
			if (guiMeshes.empty())
			{
				guiMeshes = App->resManager->GetResourceNamesList(TYPE::MESH, true);
			}
			for (int n = 0; n < guiMeshes.size(); n++)
			{
				bool is_selected = (mesh != nullptr ? HashString(mesh->GetName()) == HashString(guiMeshes[n].c_str()) : false);
				if (ImGui::Selectable(guiMeshes[n].c_str(), is_selected))
				{
					if(mesh == nullptr || mesh->GetName() != guiMeshes[n])
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
		ImGui::Checkbox("Highlighted", &highlighted);
		ImGui::ColorEdit3("Highlight color", &highlightColor[0]);
		// Material selector
		ImGui::Text("Material");
		ImGui::PushID("Material Combo");
		if (ImGui::BeginCombo("", material != nullptr ? material->GetName() : "None selected"))
		{
			if (guiMaterials.empty())
			{
				guiMaterials = App->resManager->GetResourceNamesList(TYPE::MATERIAL, true);
			}
			for (int n = 0; n < guiMaterials.size(); n++)
			{
				bool is_selected = (material != nullptr ? (HashString(material->GetName()) == HashString(guiMaterials[n].c_str())) : false);
				if (ImGui::Selectable(guiMaterials[n].c_str(), is_selected) )
				{
					if (material == nullptr || material->GetName() != guiMaterials[n])
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
				if (material != nullptr) 
				{
					App->editor->materialEditor->open = true;
					App->editor->materialEditor->material = material;
					App->editor->materialEditor->previous = new ResourceMaterial(*material);
					App->editor->materialEditor->SetCurrentTextures();

					// Update texture list
					App->editor->materialEditor->UpdateTexturesList();
				}
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
	value->AddUint("materialUID", (material != nullptr) ? material->GetUID() : 0u);
	value->AddInt("castShadows", castShadows);
	value->AddInt("useAlpha", useAlpha);
	value->AddInt("highlighted", highlighted);
	value->AddFloat3("highlightColor", highlightColor);
}

void ComponentRenderer::Load(JSON_value* value)
{
	Component::Load(value);

	unsigned meshUID = value->GetUint("meshUID");
	mesh = (ResourceMesh*)App->resManager->Get(meshUID); //Look for loaded meshes
	UpdateGameObject();

	unsigned materialUID = value->GetUint("materialUID");
	material = (ResourceMaterial*)App->resManager->Get(materialUID);
	if (materialUID == 0 || material == nullptr) SetMaterial(DEFAULTMAT); //FIXME!: Default UID should'nt be 0

	castShadows = value->GetInt("castShadows");
	useAlpha = value->GetInt("useAlpha");
	highlighted = value->GetInt("highlighted");
	highlightColor = value->GetFloat3("highlightColor");
}

void ComponentRenderer::SetMaterial(const char* materialName)
{
	// Delete previous material
	if (material != nullptr)
	{
		App->resManager->DeleteResource(material->GetUID());
	}

	if (materialName == nullptr)
	{
		material = (ResourceMaterial*)App->resManager->GetByName(DEFAULTMAT,TYPE::MATERIAL);
		return;
	}
	else
	{
		material = (ResourceMaterial*)App->resManager->GetByName(materialName, TYPE::MATERIAL);

		// Material can't be found
		if(material == nullptr)
			material = (ResourceMaterial*)App->resManager->GetByName(DEFAULTMAT, TYPE::MATERIAL);
	}
	return;
}

void ComponentRenderer::SetMesh(const char* meshfile)
{
	// Delete previous mesh
	if (mesh != nullptr)
	{
		App->resManager->DeleteResource(mesh->GetUID());
		App->scene->DeleteFromSpacePartition(gameobject);
	}

	if (meshfile != nullptr)
		mesh = (ResourceMesh*)App->resManager->GetByName(meshfile, TYPE::MESH);

	if (mesh != nullptr)
	{
		LinkBones();
		UpdateGameObject();
	}

	return;
}

void ComponentRenderer::UpdateGameObject()
{
	if (gameobject != nullptr && mesh != nullptr)
	{
		gameobject->UpdateBBox();
		App->scene->AddToSpacePartition(gameobject);
	}
}

void ComponentRenderer::LinkBones()
{
	bindBones.clear();
	bindBones = mesh->bindBones;

	if (bindBones.size() == 0)
	{
		return;
	}
	unsigned linkedCount = 0u;

	for (unsigned i = 0u; i < bindBones.size(); ++i)
	{
		GameObject* node = gameobject;
		while (node != nullptr && !node->isBoneRoot)
		{
			node = node->parent;
		}

		if (node == nullptr)
		{
			return;
		}

		bool found = false;

		std::stack<GameObject*> S;
		S.push(node);

		while (!S.empty() && !found)
		{
			node = S.top();
			S.pop();
			if (node->name == bindBones[i].name)
			{
				found = true;
				bindBones[i].go = node;
				++linkedCount;
			}
			else
			{
				for (GameObject* go : node->children)
				{
					S.push(go);
				}
			}
		}
	}

	LOG("Linked %d bones from %s", linkedCount, gameobject->name.c_str());

}

void ComponentRenderer::DrawMesh(unsigned shaderProgram)
{
	if (bindBones.size() > 0)
	{
		std::vector<math::float4x4> palette(bindBones.size(), math::float4x4::identity); //TODO: Declare on .h
		unsigned i = 0u;
		for (BindBone bb : bindBones)
		{
			palette[i++] = bb.go->GetGlobalTransform() * bb.transform;
		}

		glUniformMatrix4fv(glGetUniformLocation(shaderProgram,
			"palette"), bindBones.size(), GL_TRUE, palette[0].ptr());
	}

	glBindVertexArray(mesh->VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
	glDrawElements(GL_TRIANGLES, mesh->meshIndices.size(), GL_UNSIGNED_INT, 0);

	// We disable VAO
	glBindVertexArray(0);
}
