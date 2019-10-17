#include "debugdraw.h"
#include "Application.h"

#include "ModuleCamera.h"
#include "ModuleEditor.h"
#include "ModuleFileSystem.h"
#include "ModuleInput.h"
#include "ModuleProgram.h"
#include "ModuleResourceManager.h"
#include "ModuleRender.h"
#include "ModuleScene.h"
#include "ModuleTime.h"
#include "ModuleTextures.h"
#include "ModuleSpacePartitioning.h"
#include "ModuleParticles.h"
#include "ModuleWindow.h"
#include "ModuleScript.h"
#include "ModuleNavigation.h"

#include "GameObject.h"
#include "ComponentCamera.h"
#include "ComponentRenderer.h"
#include "ComponentTransform.h"
#include "ComponentText.h"
#include "ComponentTrail.h"
#include "ComponentParticles.h"
#include "ComponentVolumetricLight.h"
#include "BaseScript.h"

#include "ResourceTexture.h"
#include "ResourceMesh.h"
#include "ResourceMaterial.h"
#include "ResourcePrefab.h"
#include "ResourceScene.h"

#include "MaterialEditor.h"
#include "PanelBrowser.h"
#include "Viewport.h"

#include "HashString.h"
#include "JSON.h"
#include "myQuadTree.h"
#include "AABBTree.h"
#include "KDTree.h"


#include "Imgui.h"
#include "Geometry/LineSegment.h"
#include "Math/MathConstants.h"
#include "GL/glew.h"
#include "Brofiler.h"


#pragma warning(push)
#pragma warning(disable : 4996)  
#pragma warning(disable : 4244)  
#pragma warning(disable : 4305)  
#pragma warning(disable : 4838)  
#define PAR_SHAPES_IMPLEMENTATION
#include "par_shapes.h"
#pragma warning(pop)

#include <random>
#include <map>
#include <stack>
#include <string>

#define MAX_DEBUG_LINES 5
#define MAX_LIGHTS 4
#define DEFAULT_SPHERE_SHAPE 20
#define QUADTREE_SIZE 20

ModuleScene::ModuleScene()
{
}


ModuleScene::~ModuleScene()
{
	RELEASE(quadtree);
}

bool ModuleScene::Init(JSON * config)
{
	pcg_extras::seed_seq_from<std::random_device> seed_source;
	pcg32 rng(seed_source);
	uuid_rng = rng;
	root = new GameObject("World", 0); //Root always has uid 0
	canvas = new GameObject("Canvas", 1);
	root->InsertChild(canvas);
	int size = QUADTREE_SIZE * App->renderer->current_scale;
	AABB limit(float3(-size, 0.f, -size), float3(size, 0.f, size));
	quadtree = new myQuadTree(limit);

	JSON_value* scene = config->GetValue("scene");
	if (scene != nullptr)
	{
		primitivesUID[(unsigned)PRIMITIVES::SPHERE] = scene->GetUint("sphereUID");
		primitivesUID[(unsigned)PRIMITIVES::CUBE] = scene->GetUint("cubeUID");
		ambientColor = scene->GetColor3("ambient");
		defaultSceneUID = scene->GetUint("defaultsceneUID");

		if (scene->GetInt("sizeScene")) 
		{
			SceneSize = scene->GetInt("sizeScene");
		}
	}
	return true;
}

bool ModuleScene::Start()
{
	camera_notfound_texture = (ResourceTexture*)App->resManager->Get(NOCAMERA);
	if (defaultSceneUID > 0u)
	{
		defaultScene = (ResourceScene*)App->resManager->Get(defaultSceneUID);
		if (defaultScene != nullptr)
		{
			LoadScene(defaultScene->GetName(), SCENES);
		}
	}
	return true;
}

update_status ModuleScene::PreUpdate()
{
	if (loadScene)
	{
		LoadScene(name.c_str(), SCENES);
		loadScene = false;
	}

#ifndef GAME_BUILD
	if (!App->renderer->viewScene->hidden)
	{
		FrustumCulling(*App->camera->editorcamera->frustum);
	}
	else if(!App->renderer->viewGame->hidden && maincamera != nullptr)
	{
		FrustumCulling(*maincamera->frustum);
	}
#else
	if (maincamera != nullptr)
	{
		FrustumCulling(*maincamera->frustum);
	}
#endif
	return UPDATE_CONTINUE;
}

update_status ModuleScene::Update(float dt)
{
	BROFILER_CATEGORY("Scene Update", Profiler::Color::Green);
	root->UpdateTransforms(math::float4x4::identity);
	root->PreUpdate();
	root->Update();
	root->CheckDelete();

	/*if (photoTimer > 0)
	{
		photoTimer -= dt;
	}

	if ((App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_DOWN && App->input->GetKey(SDL_SCANCODE_Z) == KEY_REPEAT)
		|| (App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT && App->input->GetKey(SDL_SCANCODE_Z) == KEY_DOWN))
	{
		RestoreLastPhoto();
	}
	if ((App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_DOWN && App->input->GetKey(SDL_SCANCODE_Y) == KEY_REPEAT)
		|| (App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_REPEAT && App->input->GetKey(SDL_SCANCODE_Y) == KEY_DOWN))
	{
		Redo();
	}
	*/
	return UPDATE_CONTINUE;
}

bool ModuleScene::CleanUp()
{
	root->CleanUp();
	for (auto &child : root->children)
	{
		RELEASE(child);
	}
	root->children.clear();
	
	LOG("Reset volumetric AABBTree");
	App->spacePartitioning->aabbTree.Reset();
	LOG("Reset lighting AABBTree");
	App->spacePartitioning->aabbTreeLighting.Reset();

	selected = nullptr;
	maincamera = nullptr;

	if (camera_notfound_texture != nullptr)
	{
		App->resManager->DeleteResource(camera_notfound_texture->GetUID());
		camera_notfound_texture = nullptr;
	}

	lights.clear();

	// Delete temporary scene if it exists [If you close the app in play mode temporaryScene may remain]
	if (App->time->gameState == GameState::RUN)
	{
		std::string temporaryScene(RESOURCE_SCENES);
		temporaryScene += TEMPORARY_SCENE;
		temporaryScene += SCENEEXTENSION;
		ResourceScene* scene = (ResourceScene*)App->resManager->Get(temporaryScene.c_str(), TYPE::SCENE);
		if (scene != nullptr)
		{
			// Delete temporary scene
			scene->Delete();
			App->resManager->DeleteResourceFromList(scene->GetUID());
		}
	}

	//RELEASE(defaultScene);

	return true;
}

void ModuleScene::SaveConfig(JSON* config)
{
	JSON_value* scene = config->CreateValue();

	scene->AddUint("sphereUID", primitivesUID[(unsigned)PRIMITIVES::SPHERE]);
	scene->AddUint("cubeUID", primitivesUID[(unsigned)PRIMITIVES::CUBE]);
	scene->AddFloat3("ambient", ambientColor);
	scene->AddUint("defaultsceneUID", defaultScene != nullptr ? defaultScene->GetUID() : 0u);
	scene->AddInt("sizeScene", SceneSize);
	config->AddValue("scene", *scene);
}

void ModuleScene::FrustumCulling(const Frustum& frustum)
{
	staticFilteredGOs.clear();
	dynamicFilteredGOs.clear();

	Frustum camFrustum = frustum;
#ifndef GAME_BUILD
	if (maincamera != nullptr && App->renderer->useMainCameraFrustum)
	{
		camFrustum = *maincamera->frustum;
	}
#endif
	App->spacePartitioning->kDTree.GetIntersections(camFrustum, staticFilteredGOs);
	App->spacePartitioning->aabbTree.GetIntersections(camFrustum, dynamicFilteredGOs);
}

void ModuleScene::Draw(const Frustum &frustum, bool isEditor)
{
	alphaRenderers.clear();

#ifndef GAME_BUILD
	PROFILE;
	if (isEditor)
	{
		if (App->scene->selected != nullptr && App->scene->selected->isBoneRoot)
		{
			std::stack<GameObject*> S;
			S.push(App->scene->selected);
			while (!S.empty())
			{
				GameObject* node = S.top();S.pop();
				if (node->parent->transform != nullptr)
				{
					ComponentTransform*  nT = (ComponentTransform*)node->GetComponentOld(ComponentType::Transform);
					ComponentTransform*  pT = (ComponentTransform*)node->parent->GetComponentOld(ComponentType::Transform);
					dd::line(nT->GetGlobalPosition(), pT->GetGlobalPosition(), dd::colors::Red);
				}
				
				for (GameObject* go : node->children)
				{
					S.push(go);
				}
			}
		}

		if (App->renderer->aabbTreeDebug)
		{
			App->spacePartitioning->aabbTree.Draw();
		}
		if (App->renderer->kDTreeDebug)
		{
			App->spacePartitioning->kDTree.DebugDraw();
		}

		if (App->renderer->light_debug)
		{
			for (const auto &light : lights)
			{
				light->DrawDebugLight();
			}
			App->spacePartitioning->aabbTreeLighting.Draw();
		}



	}
	Frustum camFrustum = frustum;	

	if (maincamera != nullptr && App->renderer->useMainCameraFrustum)
	{
		camFrustum = *maincamera->frustum;
	}
	for (const auto &go : staticFilteredGOs)
	{
		if (camFrustum.Intersects(go->GetBoundingBox()))
		{
			ComponentRenderer* cr = (ComponentRenderer*)go->GetComponentOld(ComponentType::Renderer);
			if (cr && !cr->useAlpha)
			{
				DrawGO(*go, camFrustum, isEditor);
			}
			else
			{
				alphaRenderers.push_back(cr);
			}
		}
	}

	for (const auto &go : dynamicFilteredGOs)
	{
		if (camFrustum.Intersects(go->GetBoundingBox()))
		{
			ComponentRenderer* cr = (ComponentRenderer*)go->GetComponentOld(ComponentType::Renderer);
			if (!cr)
			{
				cr = ((ComponentVolumetricLight*)go->GetComponentOld(ComponentType::VolumetricLight))->renderer;
			}
			if (cr && !cr->useAlpha)
			{
				DrawGO(*go, camFrustum, isEditor);
			}
			else
			{
				alphaRenderers.push_back(cr);
			}
		}
	}

#else
	for (const auto &go : staticFilteredGOs)
	{
		if (maincamera->frustum->Intersects(go->GetBoundingBox()))
		{
			ComponentRenderer* cr = (ComponentRenderer*)go->GetComponentOld(ComponentType::Renderer);
			if (cr && !cr->useAlpha)
			{
				DrawGO(*go, *App->scene->maincamera->frustum);
			}
			else
			{
				alphaRenderers.push_back(cr);
			}
		}
	}

	for (const auto &go : dynamicFilteredGOs)
	{
		if (maincamera->frustum->Intersects(go->GetBoundingBox()))
		{
			ComponentRenderer* cr = (ComponentRenderer*)go->GetComponentOld(ComponentType::Renderer);
			if (!cr)
			{
				cr = ((ComponentVolumetricLight*)go->GetComponentOld(ComponentType::VolumetricLight))->renderer;
			}
			if (cr && !cr->useAlpha)
			{
				DrawGO(*go, *App->scene->maincamera->frustum);
			}
			else
			{
				alphaRenderers.push_back(cr);
			}
		}
	}	

#endif

	alphaRenderers.insert(alphaRenderers.end(), App->particles->particleSystems.begin(), App->particles->particleSystems.end());
	alphaRenderers.insert(alphaRenderers.end(), App->particles->trails.begin(), App->particles->trails.end());

	alphaRenderers.sort(
		[frustum](const Component* cr1, const Component* cr2) -> bool
	{
		return cr1->gameobject->transform->GetGlobalPosition().Distance(frustum.pos) > cr2->gameobject->transform->GetGlobalPosition().Distance(frustum.pos);
	});
	
	ComponentCamera* camera = isEditor ? App->camera->editorcamera : App->scene->maincamera;

	assert(camera);

	if (alphaRenderers.size() > 0)
	{
		glEnable(GL_BLEND);
		
		for (Component* cr : alphaRenderers)
		{
			if (!cr->enabled) continue;

			switch (cr->type)
			{
				case ComponentType::Renderer:
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#ifndef GAME_BUILD

					DrawGO(*cr->gameobject, camFrustum, isEditor);
#else
					DrawGO(*cr->gameobject, *camera->frustum);
#endif
					break;
				case ComponentType::Trail:
				{
					ComponentTrail* trail = (ComponentTrail*)cr;
					glDisable(GL_CULL_FACE);
					glBlendFunc(GL_ONE, GL_ONE);

					trail->UpdateTrail();
					if (trail->trail.size() > 1)
					{
						App->particles->RenderTrail(trail, camera);
					}

					glEnable(GL_CULL_FACE);
					break;
				}
				case ComponentType::Particles:
				{
					ComponentParticles* particles = (ComponentParticles*)cr;
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#ifndef GAME_BUILD
					particles->Update(App->time->gameDeltaTime, camFrustum.pos);
#else
					particles->Update(App->time->gameDeltaTime, camera->frustum->pos);
#endif
					App->particles->DrawParticleSystem(particles, camera);
					break;
				}
			}
		}
		glDisable(GL_BLEND);
	}
}

void ModuleScene::DrawGO(const GameObject& go, const Frustum & frustum, bool isEditor)
{
	PROFILE;
	if (!go.isActive()) return;

	if (go.drawBBox && isEditor)
	{
		go.DrawBBox();
		if (go.light != nullptr)
		{
			go.light->DrawDebug();
		}
	}
	ComponentRenderer* crenderer = (ComponentRenderer*)go.GetComponentOld(ComponentType::Renderer);
	if (crenderer == nullptr || !crenderer->enabled || crenderer->material == nullptr)
	{
		ComponentVolumetricLight* cVL = (ComponentVolumetricLight*)go.GetComponentOld(ComponentType::VolumetricLight);		
		if (!cVL)
			return;
		crenderer = cVL->renderer;
	}
	ResourceMesh* mesh = crenderer->mesh;
	ResourceMaterial* material = crenderer->material;
	Shader* shader = material->shader;
	if (shader == nullptr) return;
	
	unsigned variation = 0u;
	if (shader->id.size() > 1) //If exists variations use it
	{
		variation = material->variation;
		if (crenderer->water)
		{
			variation |= (unsigned)ModuleProgram::PBR_Variations::WATER;
		}
		else
		{
			if (mesh != nullptr && mesh->bindBones.size() > 0 && !crenderer->avoidSkinning)
			{
				variation |= (unsigned)ModuleProgram::PBR_Variations::SKINNED;
			}
			if (App->renderer->directionalLight && App->renderer->directionalLight->produceShadows)
			{
				variation |= (unsigned)ModuleProgram::PBR_Variations::SHADOWS_ENABLED;
			}
			if (isEditor)
			{
				variation |= (unsigned)ModuleProgram::PBR_Variations::EDITOR_RENDER;
			}
			if (crenderer->dissolve)
			{
				variation |= (unsigned)ModuleProgram::PBR_Variations::DISSOLVE;
			}
		}
	}

	glUseProgram(shader->id[variation]);
	  
	material->SetUniforms(shader->id[variation], shader->isFX, crenderer);

	glUniform3fv(glGetUniformLocation(shader->id[variation],
		"lights.ambient_color"), 1, (GLfloat*)&ambientColor);
	
	glUniform1f(glGetUniformLocation(shader->id[variation],
		"time"), App->time->realTime * crenderer->waterSpeed);

	glUniform1f(glGetUniformLocation(shader->id[variation],
		"waterAmplitude1"), crenderer->waterAmplitude1);

	glUniform1f(glGetUniformLocation(shader->id[variation],
		"decay1"), crenderer->waterDecay1);

	glUniform1f(glGetUniformLocation(shader->id[variation],
		"frequency1"), crenderer->waterFrequency1);

	glUniform3fv(glGetUniformLocation(shader->id[variation],
		"source1"), 1, (GLfloat*)&crenderer->waterSource1);

	glUniform1f(glGetUniformLocation(shader->id[variation],
		"waterAmplitude2"), crenderer->waterAmplitude2);

	glUniform1f(glGetUniformLocation(shader->id[variation],
		"decay2"), crenderer->waterDecay2);

	float waterMix = sin(App->time->gameTime * crenderer->distorsionSpeed);
	waterMix = waterMix < 0.f ? -waterMix : waterMix;

	glUniform1f(glGetUniformLocation(shader->id[variation],
		"waterMix"), waterMix);

	glUniform2fv(glGetUniformLocation(shader->id[variation],
		"uvScaler"), 1, (GLfloat*)&crenderer->uvScaler);
	
	glUniform1f(glGetUniformLocation(shader->id[variation],
		"frequency2"), crenderer->waterFrequency2);

	glUniform3fv(glGetUniformLocation(shader->id[variation],
		"source2"), 1, (GLfloat*)&crenderer->waterSource2);

	glUniform3fv(glGetUniformLocation(shader->id[variation],
		"eyePosUniform"), 1, (GLfloat*)&frustum.pos);

	glUniform1f(glGetUniformLocation(shader->id[variation], "sliceAmount"), crenderer->dissolveAmount);
	glUniform1f(glGetUniformLocation(shader->id[variation], "borderAmount"), crenderer->borderAmount);

	go.SetLightUniforms(shader->id[variation]);

	go.UpdateModel(shader->id[variation]);
	
	if (crenderer->highlighted)
	{
		glUniform3fv(glGetUniformLocation(shader->id[variation],
			"highlightColorUniform"), 1, (GLfloat*)&crenderer->highlightColor);
	}
	else
	{
		float zero[] = { .0f, .0f, .0f };
		glUniform3fv(glGetUniformLocation(shader->id[variation],
			"highlightColorUniform"), 1, (GLfloat*) zero);
	}
	
	if (mesh != nullptr)
	{
		crenderer->DrawMesh(shader->id[variation]);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glUseProgram(0);
}

void ModuleScene::DrawHierarchy()
{
	ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.2f, 0.5f, 1.00f));
	root->DrawHierarchy();
	ImGui::PopStyleColor();
}

void ModuleScene::DragNDropMove(GameObject* target) 
{
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DragDropHierarchy"))
		{
			IM_ASSERT(payload->DataSize == sizeof(GameObject*));
			TakePhoto();
			GameObject* droppedGo = (GameObject *)*(const int*)payload->Data;
			if (droppedGo != App->scene->root && target != App->scene->root && target != droppedGo && !droppedGo->IsParented(*target->parent))
			{
				if (std::find(selection.begin(), selection.end(), droppedGo) != selection.end())
				{
					for (GameObject* droppedGo : App->scene->selection)
					{
						if (droppedGo->UUID > 1)
						{
							droppedGo->parent->children.remove(droppedGo);

							std::list<GameObject*>::iterator it = std::find(target->parent->children.begin(), target->parent->children.end(), target);

							target->parent->children.insert(it, droppedGo);

							if (droppedGo->transform != nullptr)
							{
								droppedGo->transform->SetLocalToWorld();
							}

							droppedGo->parent = target->parent;
							if (droppedGo->transform != nullptr)
							{
								droppedGo->transform->SetWorldToLocal(droppedGo->parent->GetGlobalTransform());
							}
						}
					}
				}
				else
				{
					ResourcePrefab* prefab = (ResourcePrefab*)App->resManager->Get(droppedGo->prefabUID); //We generate the gameobject itself
					droppedGo->CleanUp();
					RELEASE(droppedGo);
					droppedGo = new GameObject(*prefab->RetrievePrefab());
					App->resManager->DeleteResource(prefab->GetUID());

					if (droppedGo->UUID > 1)
					{
						std::list<GameObject*>::iterator it = std::find(target->parent->children.begin(), target->parent->children.end(), target);
						target->parent->children.insert(it, droppedGo);
						if (droppedGo->transform != nullptr)
						{
							droppedGo->transform->SetLocalToWorld();
						}

						droppedGo->parent = target->parent;
						if (droppedGo->transform != nullptr)
						{
							droppedGo->transform->SetWorldToLocal(droppedGo->parent->GetGlobalTransform());
						}

						droppedGo->LinkBones();
					}
					App->editor->assets->ResetDragNDrop();
				}
			}
		}
		ImGui::EndDragDropTarget();
	}
}

void ModuleScene::DragNDrop(GameObject* go)
{
	if (go->UUID > 1 && ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	{
		ImGui::SetDragDropPayload("DragDropHierarchy", &go, sizeof(GameObject *), ImGuiCond_Once);
		if (std::find(selection.begin(), selection.end(), go) == selection.end())
		{
			ImGui::Text("%s", go->name.c_str());
		}
		else
		{
			for (GameObject* selectionGO : selection)
				ImGui::Text("%s", selectionGO->name.c_str());
		}
		ImGui::EndDragDropSource();
	}
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DragDropHierarchy"))
		{
			IM_ASSERT(payload->DataSize == sizeof(GameObject*));
			GameObject* droppedGo = (GameObject *)*(const int*)payload->Data;
			if (droppedGo != root && droppedGo->parent != go && !droppedGo->IsParented(*go) 
				&& (std::find(selection.begin(), selection.end(), droppedGo) == selection.end() ||
					std::find(selection.begin(), selection.end(), go) == selection.end())
				&& (!droppedGo->isPrefab || (!go->isPrefab && !go->ParentPrefab())))
			{
				TakePhoto();
				if (std::find(selection.begin(), selection.end(), droppedGo) != selection.end()) //Case dragNdrop in hierarchy
				{
					for (GameObject* selectedGo : selection)
					{
						if (selectedGo->UUID > 1)
						{
							go->children.push_back(selectedGo);

							if (selectedGo->transform != nullptr)
							{
								selectedGo->transform->SetLocalToWorld();
							}
							selectedGo->parent->children.remove(selectedGo);
							selectedGo->parent = go;
							if (selectedGo->transform != nullptr)
							{
								selectedGo->transform->SetWorldToLocal(selectedGo->parent->GetGlobalTransform());
							}
						}
					}
				}
				else //case dragNdrop prefabs from assets
				{
					ResourcePrefab* prefab = (ResourcePrefab*)App->resManager->Get(droppedGo->prefabUID); //We generate the gameobject itself
					droppedGo->CleanUp();
					RELEASE(droppedGo);
					droppedGo = new GameObject(*prefab->RetrievePrefab());
					App->resManager->DeleteResource(prefab->GetUID());

					if (droppedGo->UUID > 1) 
					{
						go->children.push_back(droppedGo);

						if (droppedGo->transform != nullptr)
						{
							droppedGo->transform->SetLocalToWorld();
						}
						droppedGo->parent = go;
						if (droppedGo->transform != nullptr)
						{
							droppedGo->transform->SetWorldToLocal(droppedGo->parent->GetGlobalTransform());
						}
						droppedGo->LinkBones();
					}
					App->editor->assets->ResetDragNDrop();
				}
			}
		}
		ImGui::EndDragDropTarget();
	}
}

void ModuleScene::DrawGUI()
{
	ImGui::ColorEdit3("Ambient", (float*)&ambientColor);
	if (ImGui::InputInt("Scene size", &SceneSize))
	{
		if (SceneSize <= 0)
		{
			SceneSize = 1;
		}
	}
	if (ImGui::InputInt("KdTree bucket size", &App->spacePartitioning->kDTree.bucketSize))
	{
		if (App->spacePartitioning->kDTree.bucketSize <= 0)
		{
			App->spacePartitioning->kDTree.bucketSize = 1;
		}
		App->spacePartitioning->kDTree.Calculate();
	}
	if (ImGui::InputInt("KdTree max depth", &App->spacePartitioning->kDTree.maxDepth))
	{
		if (App->spacePartitioning->kDTree.maxDepth <= 0)
		{
			App->spacePartitioning->kDTree.maxDepth = 1;
		}
		App->spacePartitioning->kDTree.Calculate();
	}
	if(ImGui::Button("Reset kdTree"))
	{
		App->spacePartitioning->kDTree.CleanUp();
		App->spacePartitioning->kDTree.Init();
		App->spacePartitioning->kDTree.Calculate();
	}

	// Default Scene selection
	if (sceneFiles.empty())
		UpdateScenesList();

	if (ImGui::BeginCombo("Default scene", defaultScene != nullptr ? defaultScene->GetName() : None))
	{
		bool none_selected = (defaultScene == nullptr);
		if (ImGui::Selectable(None, none_selected))
		{
			if (defaultScene != nullptr)
			{
				App->resManager->DeleteResource(defaultScene->GetUID());
				defaultScene = nullptr;
			}
		}
		if (none_selected)
			ImGui::SetItemDefaultFocus();
		for (int n = 0; n < sceneFiles.size(); n++)
		{
			bool is_selected = (defaultScene != nullptr && (HashString(defaultScene->GetName()) == HashString(sceneFiles[n].c_str())));
			if (ImGui::Selectable(sceneFiles[n].c_str(), is_selected) && !is_selected)
			{
				// Delete previous texture
				if (defaultScene != nullptr)
					App->resManager->DeleteResource(defaultScene->GetUID());

				defaultScene = (ResourceScene*)App->resManager->GetByName(sceneFiles[n].c_str(), TYPE::SCENE);
			}
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	if (ImGui::Button("Refresh Scenes List"))
	{
		UpdateScenesList();
	}

}

GameObject* ModuleScene::CreateGameObject(const char* name, GameObject* parent)
{
	GameObject * gameobject = new GameObject(name, GetNewUID());
	if (parent != nullptr)
	{
		gameobject->parent = parent;
		parent->children.push_back(gameobject);
	}
	return gameobject;
}

void ModuleScene::AddToSpacePartition(GameObject* gameobject)
{
	assert(gameobject != nullptr);
	if (gameobject == nullptr)	return;

	if (gameobject->isStatic)
	{
		if (gameobject->isVolumetric)
		{
			staticGOs.insert(gameobject);
		}
	}
	else
	{
		if (gameobject->isVolumetric)
		{
			App->spacePartitioning->aabbTree.InsertGO(gameobject);
		}
	}
	if (gameobject->hasLight)
	{
		App->spacePartitioning->aabbTreeLighting.InsertGO(gameobject);
	}
}

void ModuleScene::DeleteFromSpacePartition(GameObject* gameobject)
{
	if (gameobject->isStatic && gameobject->isVolumetric)
	{
		staticGOs.erase(gameobject);
		App->spacePartitioning->kDTree.Calculate();
	}
	else
	{
		if (gameobject->isVolumetric && gameobject->treeNode != nullptr)
		{
			App->spacePartitioning->aabbTree.ReleaseNode(gameobject->treeNode);
		}
		if (gameobject->hasLight && gameobject->treeNode != nullptr)
		{
			App->spacePartitioning->aabbTreeLighting.ReleaseNode(gameobject->treeNode);
		}
	}
	dynamicGOs.erase(gameobject);
	dynamicFilteredGOs.erase(gameobject);
	staticFilteredGOs.erase(gameobject);
}

void ModuleScene::ResetQuadTree() //deprecated
{
	int size = QUADTREE_SIZE * App->renderer->current_scale;
	AABB limit(float3(-size, 0.f, -size), float3(size, 0.f, size));
	quadtree->Clear(limit);
	std::stack<GameObject*> gos;
	gos.push(root);

	while (!gos.empty())
	{
		GameObject* go = gos.top();
		gos.pop();
		if (go->isStatic)
		{
			quadtree->Insert(go);
		}
	}
}

bool ModuleScene::SceneIsOlderThanPrefab(unsigned UID) const
{
	Resource* scene = App->resManager->GetByName(name.c_str(), TYPE::SCENE);
	Resource* prefab = App->resManager->GetWithoutLoad(UID);

	if (scene == nullptr || prefab == nullptr) return false;

	int prefabTime = App->fsystem->GetModTime(prefab->GetFile());
	int sceneTime = App->fsystem->GetModTime(scene->GetFile());
	App->resManager->DeleteResource(scene->GetUID());
	return prefabTime > sceneTime;
}

unsigned ModuleScene::CreatePrefab(GameObject* go)
{
	ResourcePrefab* prefab = (ResourcePrefab*)App->resManager->CreateNewResource(TYPE::PREFAB);
	std::string filepath;
	if (App->fsystem->Exists(PREFABS))
	{
		filepath = PREFABS;
		prefab->SetFile((PREFABS + go->name + PREFABEXTENSION).c_str());
	}
	else
	{
		filepath = ASSETS;
		prefab->SetFile((ASSETS + go->name + PREFABEXTENSION).c_str());
	}
	std::string exportedFile(IMPORTED_PREFABS);

	unsigned uid = prefab->GetUID();
	exportedFile += std::to_string(uid); //name
	exportedFile += PREFABEXTENSION;
	prefab->SetName(go->name.c_str());
	prefab->SetExportedFile(exportedFile.c_str());
	prefab->Save(go); //We leave the resource loaded for getting it later
	go->prefabTimeStamp = App->fsystem->GetModTime(prefab->GetFile());

	if (!App->resManager->ImportFile((go->name + PREFABEXTENSION).c_str(), filepath.c_str(), TYPE::PREFAB))
	{
		LOG("Could not import Prefab %s!!", go->name);
		return 0;
	}

	return uid;
}

void ModuleScene::CreateCube(const char * name, GameObject* parent)
{
	if (!primitivesUID[(unsigned)PRIMITIVES::CUBE])
	{
		par_shapes_mesh* mesh = par_shapes_create_cube();
		if (mesh->normals == nullptr)
		{
			par_shapes_compute_normals(mesh);
		}
		SetPrimitiveMesh(mesh, PRIMITIVES::CUBE);
	}
	CreatePrimitive(name, parent, PRIMITIVES::CUBE);
}

void ModuleScene::CreateSphere(const char * name, GameObject* parent)
{
	if (!primitivesUID[(unsigned)PRIMITIVES::SPHERE])
	{
		par_shapes_mesh* mesh = par_shapes_create_parametric_sphere(DEFAULT_SPHERE_SHAPE, DEFAULT_SPHERE_SHAPE);
		SetPrimitiveMesh(mesh, PRIMITIVES::SPHERE);
	}
	CreatePrimitive(name, parent, PRIMITIVES::SPHERE);
}

void ModuleScene::CreatePrimitive(const char * name, GameObject* parent, PRIMITIVES type)
{
	GameObject * gameobject = CreateGameObject(name, parent);
	Select(gameobject);
	ComponentTransform* transform = (ComponentTransform*)gameobject->CreateComponent(ComponentType::Transform);
	transform->scale.SetFromScalar(App->renderer->current_scale);
	transform->UpdateTransform();
	ComponentRenderer* crenderer = (ComponentRenderer*)gameobject->CreateComponent(ComponentType::Renderer);

	unsigned uid = primitivesUID[(unsigned)type];
	// TODO [MeshRefactor] - Refactor sphere/Cube creation
	//char *data = nullptr;
	//App->fsystem->Load((MESHES + std::to_string(uid) + MESHEXTENSION).c_str(), &data);
	//crenderer->UpdateMesh(data, uid);//Deallocates data
	App->resManager->Get(uid);
	crenderer->UpdateGameObject();
	crenderer->SetMaterial(DEFAULTMAT);
	//App->resManager->AddMesh(crenderer->mesh);
	App->scene->Select(gameobject);
}

void ModuleScene::SetPrimitiveMesh(par_shapes_mesh_s *mesh, PRIMITIVES type)
{
	par_shapes_scale(mesh, 1.f, 1.f, 1.f);
	char* data = nullptr;
	unsigned meshSize = SaveParShapesMesh(*mesh, &data);
	unsigned uid = GetNewUID();
	bool saved = App->fsystem->Save((MESHES + std::to_string(uid) + MESHEXTENSION).c_str(), data, meshSize);
	if (saved)
	{
		LOG("Primitive mesh %u saved", uid);
	}
	else
	{
		LOG("Error saving primitive mesh %u", uid);
	}
	primitivesUID[(unsigned)type] = uid;
	par_shapes_free_mesh(mesh);
}

unsigned ModuleScene::SaveParShapesMesh(const par_shapes_mesh_s &mesh, char** data) const //TODO: unify somehow with importer
{
	unsigned size = 0;
	unsigned ranges[2] = { mesh.ntriangles*3, mesh.npoints};
	size += sizeof(ranges); //numfaces + numvertices
	size += ranges[0] * 3 * sizeof(int); //indices
	
	size += sizeof(float)*ranges[1] * 3;
	size += sizeof(bool) * 2; //has normals + has tcoords
	if (mesh.normals != nullptr)
	{
		size += sizeof(float)*ranges[1] * 3;
	}
	if (mesh.tcoords != nullptr)
	{
		size += sizeof(float)*ranges[1] * 2;
	}

	*data = new char[size];
	char *cursor = *data;

	unsigned rangeBytes = sizeof(ranges); 
	memcpy(cursor, ranges, rangeBytes);
	cursor += rangeBytes;

	unsigned verticesBytes = sizeof(float)*mesh.npoints * 3;
	memcpy(cursor, mesh.points, verticesBytes);
	cursor += verticesBytes;

	bool hasNormals = mesh.normals != nullptr ? true : false;
	memcpy(cursor, &hasNormals, sizeof(bool));
	cursor += sizeof(bool);

	if (hasNormals)
	{
		unsigned normalsBytes = sizeof(float)*mesh.npoints * 3;
		memcpy(cursor, mesh.normals, normalsBytes);
		cursor += normalsBytes;
	}

	bool hasTcoords = mesh.tcoords != nullptr ? true : false;
	memcpy(cursor, &hasTcoords, sizeof(bool));
	cursor += sizeof(bool);

	if (hasTcoords)
	{
		unsigned tcoordsBytes = sizeof(float)*mesh.npoints * 2;
		memcpy(cursor, mesh.tcoords, tcoordsBytes);
		cursor += tcoordsBytes;
	}

	short mask = 0;
	for (int i = 0; i < mesh.ntriangles*3; i++)
	{
		memcpy(cursor, &mesh.triangles[i], sizeof(short)); 
		cursor += sizeof(short);
		memcpy(cursor, &mask, sizeof(short));
		cursor += sizeof(short); //implicitly converting to unsigned
	}

	return size;
}

void ModuleScene::AssignNewUUID(GameObject* go, unsigned UID)
{
	go->parentUUID = UID;
	go->UUID = go->UUID == 1 ? 1 : GetNewUID();

	for (std::list<GameObject*>::iterator it = go->children.begin(); it != go->children.end(); ++it)
	{
		AssignNewUUID((*it), go->UUID);
	}
}

void ModuleScene::TakePhoto()
{
	//App->particles->Reset();
	//TakePhoto(scenePhotos);
	//scenePhotosUndoed.clear();
}

void ModuleScene::TakePhoto(std::list<GameObject*>& target)
{
	photoTimer = TIME_BETWEEN_PHOTOS;
	photoEnabled = true;	
	target.push_back(new GameObject(*root));
	if (target.size() > MAX_PHOTOS)
	{
		RELEASE(target.front());
		target.pop_front();
	}
	photoEnabled = false;	
}
void ModuleScene::RestorePhoto(GameObject* photo)
{
	//photoTimer = 0.f;
	//root = photo;
	//root->UUID = 0; // Restore root UUID
	//root->children.front()->UUID = 1; //Restore canvas UUID
	//std::stack<GameObject*> goStack;
	//goStack.push(root);
	//App->renderer->directionalLight = nullptr;
	//App->particles->Reset();
	//while (!goStack.empty())
	//{
	//	GameObject* go = goStack.top(); goStack.pop();

	//	for (Component* comp : go->components)
	//	{
	//		switch (comp->type)
	//		{
	//		case ComponentType::Renderer:
	//		{
	//			if (!go->isStatic)
	//			{
	//				App->spacePartitioning->aabbTree.InsertGO(go);
	//			}
	//			else
	//			{
	//				staticGOs.insert(go);
	//				App->spacePartitioning->kDTree.Calculate();
	//			}
	//			go->isVolumetric = true;
	//			ComponentRenderer* cr = (ComponentRenderer*)go->GetComponent(ComponentType::Renderer);
	//			cr->LinkBones();
	//			break;
	//		}
	//		case ComponentType::Light:
	//			go->light = (ComponentLight*)comp;
	//			go->light->CalculateGuizmos();
	//			App->spacePartitioning->aabbTreeLighting.InsertGO(go);
	//			go->hasLight = true;
	//			lights.push_back((ComponentLight*)comp);
	//			if (go->light->lightType == LightType::DIRECTIONAL)
	//			{
	//				App->renderer->directionalLight = go->light;
	//			}
	//			break;
	//		case ComponentType::Camera:
	//			if (((ComponentCamera*)comp)->isMainClone)
	//			{
	//				maincamera = (ComponentCamera*)comp;
	//			}
	//			break;
	//		}
	//	}

	//	for (GameObject* child : go->children)
	//	{
	//		goStack.push(child);
	//	}
	//	if (go->transform != nullptr)
	//	{
	//		go->transform->UpdateTransform();
	//	}
	//}
}

void ModuleScene::RestoreLastPhoto()
{
	//if (App->scene->scenePhotos.size() > 0)
	//{
	//	TakePhoto(scenePhotosUndoed);
	//	ClearScene();
	//	RestorePhoto(scenePhotos.back());	
	//	scenePhotos.pop_back();
	//}
}

void ModuleScene::Redo()
{
	//if (scenePhotosUndoed.size() > 0)
	//{
	//	TakePhoto(scenePhotos);
	//	ClearScene();
	//	RestorePhoto(scenePhotosUndoed.back());
	//	scenePhotosUndoed.pop_back();
	//}
}

void ModuleScene::ClearScene()
{
	CleanUp();
	camera_notfound_texture = (ResourceTexture*)App->resManager->Get(NOCAMERA);
	staticGOs.clear();
	dynamicGOs.clear();
	staticFilteredGOs.clear();
	dynamicFilteredGOs.clear();
	lights.clear();
	App->renderer->directionalLight = nullptr;
	debuglines.clear();
	selection.clear();
	LOG("Reset volumetric AABBTree");
	App->spacePartitioning->aabbTree.Reset();
	LOG("Reset lighting AABBTree");
	App->spacePartitioning->aabbTreeLighting.Reset();
	App->spacePartitioning->kDTree.Calculate();
	canvas = new GameObject("Canvas", 1);
	root->InsertChild(canvas);
	App->particles->CleanUp();
	App->particles->Start();
	App->renderer->shadowCasters.clear();
}

void ModuleScene::UpdateScenesList()
{
	sceneFiles.clear();
	sceneFiles = App->resManager->GetResourceNamesList(TYPE::SCENE, true);
}

void ModuleScene::SaveScene(const GameObject& rootGO, const char* sceneName, const char* folder, bool selected)
{
	std::string sceneInAssets(folder);
	sceneInAssets += sceneName;
	sceneInAssets += SCENEEXTENSION;
	unsigned sceneUID = App->resManager->FindByFileInAssets(sceneInAssets.c_str());

	if (sceneUID != 0)
	{
		// Updating already created scene
		ResourceScene* scene = (ResourceScene*)App->resManager->GetWithoutLoad(sceneUID);
		scene->Save(rootGO, selected);
	}	
	else
	{
		// Is a new scene, create resource
		ResourceScene* scene = (ResourceScene*)App->resManager->CreateNewResource(TYPE::SCENE);
		scene->SetFile(sceneInAssets.c_str());
		std::string exportedFile(IMPORTED_SCENES);
		exportedFile += std::to_string(scene->GetUID());
		exportedFile += SCENEEXTENSION;
		scene->SetExportedFile(exportedFile.c_str());
		scene->SetName(sceneName);
		scene->Save(rootGO);
	}

	// Update scene info
	if(sceneName != TEMPORARY_SCENE)
	{
		name = sceneName;
		path = folder;
	}
}

void ModuleScene::SaveTemporaryScene()
{
	std::string temporaryScene(RESOURCE_SCENES);
	temporaryScene += TEMPORARY_SCENE;
	temporaryScene += SCENEEXTENSION;
	unsigned sceneUID = App->resManager->FindByExportedFile(temporaryScene.c_str(), TYPE::SCENE);

	// See if there is already a temporaryScene on resource list
	if (sceneUID != 0)
	{
		// Updating already saved scene [Shouldn't happen because temporaryScene is deleted after load]
		ResourceScene* scene = (ResourceScene*)App->resManager->GetWithoutLoad(sceneUID);
		scene->Save(*root);
	}
	else
	{
		// Is a new scene, create resource
		ResourceScene* scene = (ResourceScene*)App->resManager->CreateNewResource(TYPE::SCENE);
		scene->SetUsedByEngine(true);
		scene->SetExportedFile(temporaryScene.c_str());
		scene->SetFile(temporaryScene.c_str());
		scene->SetName(TEMPORARY_SCENE);
		scene->Save(*root);
	}
}

bool ModuleScene::isCleared()
{
	return App->scene->root->children.size() <= 1 &&
		App->scene->canvas->children.empty();
}

void ModuleScene::LoadScene(const char* sceneName, const char* folder)
{
	if (!isCleared())
	{
		ClearScene();
	}
	std::string oldpath(path);
	std::string oldname(name);
	if (sceneName != TEMPORARY_SCENE)
	{
		name = sceneName;
		path = folder;
	}
	if (!AddScene(sceneName, folder))
	{
		name = oldpath;
		path = oldname;
	}
	App->spacePartitioning->kDTree.Calculate();
	App->scripting->onStart = true;
	scenePhotos.clear();
	App->time->ResetGameDetaTime();
	App->renderer->OnResize();
}

void ModuleScene::LoadTemporaryScene()
{
	if (!isCleared())
	{
		ClearScene();
	}

	std::string temporaryScene(RESOURCE_SCENES);
	temporaryScene += TEMPORARY_SCENE;
	temporaryScene += SCENEEXTENSION;
	ResourceScene* scene = (ResourceScene*)App->resManager->Get(temporaryScene.c_str(), TYPE::SCENE);
	if (scene == nullptr || !scene->Load())
	{
		LOG("Error loading temporary scene.");
	}
	else
	{
		// Delete temporary scene
		scene->Delete();
		App->resManager->DeleteResourceFromList(scene->GetUID());
	}
	App->renderer->OnResize();

	App->spacePartitioning->kDTree.Calculate();
	App->scripting->onStart = true;
	scenePhotos.clear();
	App->time->ResetGameDetaTime();
}

bool ModuleScene::AddScene(const char* sceneName, const char* folder)
{
	ResourceScene* scene = (ResourceScene*)App->resManager->GetByName(sceneName, TYPE::SCENE);
	App->navigation->sceneName = sceneName;
	if(scene != nullptr && !scene->Load())
	{
		LOG("Error loading scene named: %s", sceneName);
		return false;
	}
	App->renderer->OnResize();
	return true;
}

void ModuleScene::Select(GameObject * gameobject)
{
	if (App->editor->materialEditor->material != nullptr)
	{
		App->editor->materialEditor->Save();
		App->editor->materialEditor->open = false;
	}
	if (App->input->IsKeyPressed(SDLK_LCTRL))
	{
		std::list<GameObject*>::iterator it = std::find(selection.begin(), selection.end(), gameobject);
		if (it == selection.end())
		{
			selection.push_back(gameobject);
			selected = gameobject;
			selected->isSelected = true;
			App->editor->ShowInspector();
		}
		else
		{
			(*it)->isSelected = false;
			selection.erase(it);
		}		
	}
	else
	{
		for (GameObject* go : App->scene->selection)
		{
			go->isSelected = false;
		}
		selection.clear();
		selection.push_back(gameobject);
		selected = gameobject;
		selected->isSelected = true;
		App->editor->ShowInspector();
	}
	
}

void ModuleScene::UnSelect()
{
	if (selected != nullptr)
	{
		selected->drawBBox = false;
	}
	selected = nullptr;
}

void ModuleScene::Pick(float normalized_x, float normalized_y)
{
	LineSegment line = App->camera->editorcamera->DrawRay(normalized_x, normalized_y);
	std::list<std::pair<float, GameObject*>> GOs = GetStaticIntersections(line);
	std::list<std::pair<float, GameObject*>> dGOs = GetDynamicIntersections(line);
	GOs.merge(dGOs);

	float closestTriangle = FLOAT_INF;
	GameObject * closestGO = nullptr;

	for (const auto& go : GOs)
	{
		if (closestGO != nullptr && !go.second->GetBoundingBox().Intersects(closestGO->GetBoundingBox())) //if no intersection between AABB no need to check triangles
		{
			break;
		}
		else
		{
			float distance = FLOAT_INF;
			if (go.second->Intersects(line, distance)) //returns distance to line if triangle hit
			{
				if (distance < closestTriangle)
				{
					closestGO = go.second;
					closestTriangle = distance;
				}
			}
		}
	}

	if (closestGO != nullptr)
	{
		GameObject* closestGoForReal = nullptr;
		closestGoForReal = FindClosestParent(closestGO);
		if(closestGoForReal != nullptr)
		{
			Select(closestGoForReal);
		}
		else
		{
			Select(closestGO);
		}
	
	}
	else
	{
		UnSelect();
	}

	debuglines.push_back(line);
	if (debuglines.size() > MAX_DEBUG_LINES)
	{
		debuglines.erase(debuglines.begin());
	}
}

bool ModuleScene::Intersects(math::float3& closestPoint, const char* name, bool editor)
{
	float2 mouse((float*)&App->input->GetMousePosition());
	LineSegment line;

	float normalized_x, normalized_y;

	if (editor)
	{
		math::float2 pos = App->renderer->viewScene->winPos;
		math::float2 size(App->renderer->viewScene->current_width, App->renderer->viewScene->current_height);
		normalized_x = ((mouse.x - pos.x) / size.x) * 2 - 1; //0 to 1 -> -1 to 1
		normalized_y = (1 - (mouse.y - pos.y) / size.y) * 2 - 1; //0 to 1 -> -1 to 1

		line = App->camera->editorcamera->DrawRay(normalized_x, normalized_y);
	}
	else
	{
#ifndef GAME_BUILD
		math::float2 pos = App->renderer->viewGame->winPos;
		math::float2 size(App->renderer->viewGame->current_width, App->renderer->viewGame->current_height);
#else
		math::float2 pos = math::float2::zero;
		math::float2 size(App->window->width, App->window->height);
#endif
		normalized_x = ((mouse.x - pos.x) / size.x) * 2 - 1; //0 to 1 -> -1 to 1
		normalized_y = (1 - (mouse.y - pos.y) / size.y) * 2 - 1; //0 to 1 -> -1 to 1

		line = App->scene->maincamera->DrawRay(normalized_x, normalized_y);
	}
	debuglines.push_back(line);
	std::list<std::pair<float, GameObject*>> GOs = GetStaticIntersections(line);
	std::list<std::pair<float, GameObject*>> dGOs = GetDynamicIntersections(line);
	GOs.merge(dGOs);

	math::float3 intersectionPoint = math::float3::zero;
	float closestTriangle = FLOAT_INF;
	bool intersects = false;

	for (const auto& go : GOs)
	{
		if (go.second->name != name) continue;

		float distance = FLOAT_INF;
		if (go.second->Intersects(line, distance, &intersectionPoint)) //returns distance to line if triangle hit
		{
			intersects = true;
			if (distance < closestTriangle)
			{
				closestPoint = intersectionPoint;
				closestTriangle = distance;
			}
		}
	}

	return intersects;
}

bool ModuleScene::Intersects(const char* tag, bool sorted, math::float3& point, 
	GameObject** out) const
{
	float2 mouse((float*)&App->input->GetMousePosition());
	LineSegment line;

	float normalized_x, normalized_y;

	if (App->renderer->viewGame != nullptr && App->renderer->viewGame->hidden)
	{
		math::float2 pos = App->renderer->viewScene->winPos;
		math::float2 size(App->renderer->viewScene->current_width, App->renderer->viewScene->current_height);
		normalized_x = ((mouse.x - pos.x) / size.x) * 2 - 1; //0 to 1 -> -1 to 1
		normalized_y = (1 - (mouse.y - pos.y) / size.y) * 2 - 1; //0 to 1 -> -1 to 1

		line = App->camera->editorcamera->DrawRay(normalized_x, normalized_y);
	}
	else
	{
#ifndef GAME_BUILD
		math::float2 pos = App->renderer->viewGame->winPos;
		math::float2 size(App->renderer->viewGame->current_width, App->renderer->viewGame->current_height);
#else
		math::float2 pos = math::float2::zero;
		math::float2 size(App->window->width, App->window->height);
#endif
		normalized_x = ((mouse.x - pos.x) / size.x) * 2 - 1; //0 to 1 -> -1 to 1
		normalized_y = (1 - (mouse.y - pos.y) / size.y) * 2 - 1; //0 to 1 -> -1 to 1

		line = App->scene->maincamera->DrawRay(normalized_x, normalized_y);

	}
		std::list<std::pair<float, GameObject*>> GOs = GetStaticIntersections(line);
		std::list<std::pair<float, GameObject*>> dGOs = GetDynamicIntersections(line);
		GOs.merge(dGOs);

		math::float3 intersectionPoint = math::float3::zero;
		float closestTriangle = FLOAT_INF;
		bool intersects = false;

		for (const auto& go : GOs)
		{
			if (go.second->tag != tag) continue;

			float distance = FLOAT_INF;
			if (go.second->Intersects(line, distance, &intersectionPoint)) //returns distance to line if triangle hit
			{
				intersects = true;
				if (distance < closestTriangle)
				{
					point = intersectionPoint;
					closestTriangle = distance;
					*out = go.second;
					if (!sorted) break;
				}
			}
		}
		return intersects;
}

GameObject* ModuleScene::FindClosestParent(GameObject* go)
{
	if (go->parent != nullptr)
	{
		if (go->parent->isBoneRoot == true)
		{
			return go->parent;
		}
	}
	else
	{
		return nullptr;
	}
	return FindClosestParent(go->parent);
}

GameObject* ModuleScene::FindGameObjectByTag(const char* tag, GameObject* parent) const
{
	std::stack<GameObject*> GOs;
	if (parent == nullptr)
	{
		parent = root;
	}
	GOs.push(parent);
	while (!GOs.empty())
	{
		GameObject* go = GOs.top();
		if (go->tag == tag)
		{
			return go;
		}

		GOs.pop();
		for (const auto& child : go->children)
		{
			GOs.push(child);
		}
	}
	return nullptr;
}

std::vector<GameObject*> ModuleScene::FindGameObjectsByTag(const char* tag, GameObject* parent) const
{
	std::vector<GameObject*> gameobjects;

	std::stack<GameObject*> GOs;
	if (parent == nullptr)
	{
		parent = root;
	}
	GOs.push(parent);
	while (!GOs.empty())
	{
		GameObject* go = GOs.top();
		if (go->tag == tag)
		{
			gameobjects.push_back(go);
		}

		GOs.pop();
		for (const auto& child : go->children)
		{
			GOs.push(child);
		}
	}
	return gameobjects;
}

GameObject* ModuleScene::FindGameObjectByUID(unsigned UID, GameObject* parent) const
{
	std::stack<GameObject*> GOs;
	if (parent == nullptr)
	{
		GOs.push(root);
	}
	else
	{
		GOs.push(parent);
	}
	while (!GOs.empty())
	{
		GameObject* go = GOs.top();
		if (go->UUID == UID)
		{
			return go;
		}

		GOs.pop();
		for (const auto& child : go->children)
		{
			GOs.push(child);
		}
	}
	return nullptr;
}

GameObject* ModuleScene::FindGameObjectByName(const char* name, GameObject* parent) const
{
	std::stack<GameObject*> GOs;
	if (parent == nullptr)
	{
		GOs.push(root);
	}
	else
	{
		GOs.push(parent);
	}
	while (!GOs.empty())
	{
		GameObject* go = GOs.top();
		if (go->name == std::string(name))
		{
			return go;
		}

		GOs.pop();
		for (const auto& child : go->children)
		{
			GOs.push(child);
		}
	}
	return nullptr;
}



GameObject* ModuleScene::Spawn(const char * name, GameObject * parent, math::float3 position)
{
	ResourcePrefab* prefab = (ResourcePrefab*) App->resManager->GetByName(name, TYPE::PREFAB);
	if (prefab == nullptr)
	{
		LOG("Prefab %s Not Found", name);
		return nullptr;
	}

	//Instantiate prefab
	GameObject* instance = new GameObject(*prefab->RetrievePrefab());
	instance->LinkBones();
	App->resManager->DeleteResource(prefab->GetUID());

	//since the spawning enemies of the naruto enemy made the crowding crash
	//its necessary having the position before initializing the EnemyControllerScript
	instance->transform->SetPosition(position);

	if (parent == nullptr)
	{
		parent = root;
	}
	parent->children.push_back(instance);
	instance->parent = parent;
	instance->transform->Reset();
	if (App->time->gameState == GameState::RUN)
	{
		instance->OnPlay();
		std::queue<GameObject*> gos;
		gos.push(instance);
		while (!gos.empty())
		{
			GameObject* go = gos.front();
			gos.pop();
			std::vector<Component*> components = go->GetComponents(ComponentType::Script);
			for (size_t i = 0; i < components.size(); i++)
			{
				if (go->isActive())
				{
					((Script*)components[i])->Awake();
					((Script*)components[i])->hasBeenAwoken = true;
				}
				if (components[i]->enabled)
				{
					((Script*)components[i])->Start();
					((Script*)components[i])->hasBeenStarted = true;
				}
			}
			for (const auto & child : go->children)
			{
					gos.push(child);
			}
		}
	}
	return instance;
}

GameObject * ModuleScene::Spawn(const char * name, math::float3 position, math::Quat rotation, GameObject * parent)
{
	GameObject* instance = Spawn(name, parent, position);
	if (instance != nullptr)
	{
		instance->transform->SetPosition(position);
		instance->transform->SetRotation(rotation);
	}
	return instance;
}

void ModuleScene::GetStaticGlobalAABB(AABB & aabb, std::vector<GameObject*>& bucket, unsigned int & bucketOccupation)
{
	aabb.SetNegativeInfinity();
	for (GameObject* go : staticGOs)
	{
		aabb.Enclose(go->bbox);
		bucket[++bucketOccupation] = go;
	}
}


unsigned ModuleScene::GetNewUID()
{
	return uuid_rng();
}

std::list<ComponentLight*> ModuleScene::GetClosestLights(LightType type, math::float3 position) const
{
	std::map<float, ComponentLight*> lightmap;
	for (const auto& light : lights)
	{
		if (light->lightType == type && light->enabled && light->gameobject->transform != nullptr)
		{
			float distance = light->gameobject->transform->GetPosition().Distance(position);
			lightmap.insert(std::pair<float, ComponentLight*>(distance,light));
		}
	}

	std::list<ComponentLight*> closest;
	int i = 0;
	for (const auto& light : lightmap)
	{
		closest.push_back(light.second);
		++i;
		if (i == MAX_LIGHTS) break;
	}
	return closest;
}

ENGINE_API ComponentLight* ModuleScene::GetDirectionalLight() const
{
	for (const auto& light : lights)
	{
		if (light->lightType == LightType::DIRECTIONAL)
		{
			return light;
		}
	}
	return nullptr;
}

void ModuleScene::DeleteDirectionalLight(ComponentLight* light)
{
	lights.remove(light);
	if (App->renderer->directionalLight == light)
	{
		App->renderer->directionalLight = nullptr;
	}
}

math::LineSegment ModuleScene::SceneRaycast(math::float2 position)
{
	float normalized_x, normalized_y;

#ifndef GAME_BUILD
		math::float2 winPos = App->renderer->viewGame->winPos;
		math::float2 size(App->renderer->viewGame->current_width, App->renderer->viewGame->current_height);
#else
		math::float2 winPos = math::float2::zero;
		math::float2 size(App->window->width, App->window->height);
#endif
		normalized_x = ((position.x - winPos.x) / size.x) * 2 - 1; //0 to 1 -> -1 to 1
		normalized_y = (1 - (position.y - winPos.y) / size.y) * 2 - 1; //0 to 1 -> -1 to 1

		return App->scene->maincamera->DrawRay(normalized_x, normalized_y);
}

std::list<GameObject*> ModuleScene::SceneRaycastHit(math::float2 position)
{
	std::list<GameObject*> go_list;
	for (std::pair<float, GameObject*> item : GetDynamicIntersections(SceneRaycast(position), false))
	{
		go_list.emplace_back(item.second);
	}

	return go_list;
}

std::list<std::pair<float, GameObject*>> ModuleScene::GetDynamicIntersections(const LineSegment & line, bool sort) const
{
	std::list<std::pair<float, GameObject*>> gos; 
	std::unordered_set<GameObject*> intersections;
	App->spacePartitioning->aabbTree.GetIntersections(line, intersections);
	for (const auto &go : intersections)
	{
		if (!go->isActive()) continue;
		float dNear = -FLOAT_INF;
		float dFar = FLOAT_INF;
		if (line.Intersects(go->GetBoundingBox(), dNear, dFar))
		{
			gos.push_back(std::pair<float, GameObject*>(dNear, go));
		}
	}
	if (sort) gos.sort();
	return gos;
}

std::list<std::pair<float, GameObject*>> ModuleScene::GetStaticIntersections(const LineSegment & line) const
{
	std::list<std::pair<float, GameObject*>> gos;
	std::unordered_set<GameObject*> intersections;
	App->spacePartitioning->kDTree.GetIntersections(line, intersections);
	for (const auto &go : intersections)
	{
		if (!go->isActive()) continue;
		float dNear = -FLOAT_INF;
		float dFar = FLOAT_INF;
		if (line.Intersects(go->GetBoundingBox(), dNear, dFar))
		{
			gos.push_back(std::pair<float, GameObject*>(dNear, go));
		}
	}
	gos.sort();
	return gos;
}