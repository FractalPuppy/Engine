#include "Application.h"

#include "ModuleRender.h"
#include "ModuleCamera.h"
#include "ModuleScene.h"
#include "ModuleProgram.h"
#include "ModuleEditor.h"
#include "ModuleWindow.h"
#include "ModuleDebugDraw.h"
#include "ModuleResourceManager.h"
#include "ModuleUI.h"
#include "ModuleFontLoader.h"
#include "ModuleDevelopmentBuildDebug.h"
#include "ModuleParticles.h"
#include "ModuleTime.h"
#include "ModuleNavigation.h"

#include "GameObject.h"
#include "ComponentCamera.h"
#include "ComponentRenderer.h"
#include "ComponentTransform.h"
#include "Viewport.h"
#include "JSON.h"

#include "Resource.h"
#include "ResourceSkybox.h"
#include "ResourceMesh.h"

#include "SDL.h"
#include "GL/glew.h"
#include "imgui.h"
#include "Math/MathFunc.h"
#include "Math/float4x4.h"
#include "Brofiler.h"
#include "debugdraw.h"

ModuleRender::ModuleRender()
{
}

// Destructor
ModuleRender::~ModuleRender()
{
	//RELEASE(skybox);
	RELEASE(viewScene);
	RELEASE(viewGame);
}

// Called before render is available
bool ModuleRender::Init(JSON * config)
{
	LOG("Creating Renderer context");

	InitSDL();
	
	glewInit();
	InitOpenGL();

	SDL_GL_SetSwapInterval((int)vsync);

#ifndef GAME_BUILD
	viewScene = new Viewport("Scene");
	viewGame = new Viewport("Game");
#endif // !GAME_BUILD

	// Set default Skybox
	skybox = (ResourceSkybox*)App->resManager->CreateNewResource(TYPE::SKYBOX);
	std::string faces[NUMFACES] = { "Resources/Imported/right.dds", "Resources/Imported/left.dds", "Resources/Imported/top.dds", "Resources/Imported/bottom.dds", "Resources/Imported/front.dds", "Resources/Imported/back.dds" };
	skybox->SetExportedFile("Default Skybox");
	skybox->SetName("Default Skybox");
	skybox->SetTextures(faces);
	skybox->SetUsedByEngine(true);
	skybox->LoadInMemory();

	JSON_value* renderer = config->GetValue("renderer");
	if (renderer == nullptr) return true;

	msaa = renderer->GetInt("msaa");
	msaa_level = renderer->GetInt("msaa_level");
	picker_debug = renderer->GetInt("picker_debug");
	light_debug = renderer->GetInt("light_debug");
	aabbTreeDebug = renderer->GetUint("aabbTreeDebug");
	grid_debug = renderer->GetInt("grid_debug");
	depthTest = renderer->GetInt("depthTest");
	wireframe = renderer->GetInt("wireframe");
	vsync = renderer->GetInt("vsync");
	useMainCameraFrustum = renderer->GetInt("frustumMainCamera");
	skybox->enabled = renderer->GetInt("skybox");
	current_scale = renderer->GetInt("current_scale");
	gammaCorrector = renderer->GetFloat("gammaCorrector", gammaCorrector);
	exposure = renderer->GetFloat("exposure", exposure);
	
	switch (current_scale)
	{
	case 1:
		item_current = 0;
		break;
	case 10:
		item_current = 1;
		break;
	case 100:
		item_current = 2;
		break;
	}

	glGenTextures(1, &highlightBufferGame);
	glGenTextures(1, &renderedSceneGame);
	glGenTextures(1, &brightnessBufferGame);
	glGenTextures(1, &depthTexture);
	glGenTextures(2, pingpongColorbuffers);

	glGenFramebuffers(2, pingpongFBO);

	float quadVertices[] =
	{
		-1.f, 1.f, 0.f,
		-1.f, -1.f, 0.f,
		1.f, -1.f, 0.f,
		1.f, 1.f, 0.f,

		0.f, 1.f,
		0.f, 0.f,
		1.f, 0.f,
		1.f, 1.f
	};

	unsigned int quadIndices[] =
	{
		0, 1, 2,
		0, 2, 3
	};

	if (wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}


	if (postprocessVAO == 0)
	{
		glGenVertexArrays(1, &postprocessVAO);
		glGenBuffers(1, &postprocessVBO);
		glGenBuffers(1, &postprocessEBO);

		glBindVertexArray(postprocessVAO);

		glBindBuffer(GL_ARRAY_BUFFER, postprocessVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, postprocessEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(float) * 3 * 4));

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
#ifdef GAME_BUILD
	OnResize();
#endif
	return true;
}

bool ModuleRender::Start()
{
	GenBlockUniforms();
	shadowsFrustum.type = math::FrustumType::OrthographicFrustum;
	glGenFramebuffers(1, &shadowsFBO);
	glGenTextures(1, &shadowsTex);
	shadowsShader = App->program->GetProgram("Shadows");
	postProcessShader = App->program->GetProgram("PostProcess");
	blur = App->program->GetProgram("Bloom");
	return shadowsShader && shadowsFBO > 0u && shadowsTex > 0u;
}


update_status ModuleRender::PreUpdate()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	return UPDATE_CONTINUE;
}

// Called every draw update
update_status ModuleRender::Update(float dt)
{
	return UPDATE_CONTINUE;
}


update_status ModuleRender::PostUpdate()
{
	BROFILER_CATEGORY("Render PostUpdate", Profiler::Color::Black);
	ComputeShadows();

#ifndef GAME_BUILD
	viewGame->Draw(App->scene->maincamera);
	viewScene->Draw(App->camera->editorcamera, true);
	App->editor->RenderGUI();
#else
	if (App->scene->maincamera != nullptr)
	{
		App->renderer->Draw(*App->scene->maincamera, App->window->width, App->window->height, false);
	}
	App->developDebug->RenderGUI();
#endif
	SDL_GL_SwapWindow(App->window->window);

	return UPDATE_CONTINUE;
}

void ModuleRender::SaveConfig(JSON * config)
{
	JSON_value* renderer = config->CreateValue();

	renderer->AddInt("msaa", msaa);
	renderer->AddInt("msaa_level", msaa_level);
	renderer->AddInt("picker_debug", picker_debug);
	renderer->AddInt("light_debug", light_debug);
	renderer->AddInt("aabbTreeDebug", aabbTreeDebug);
	renderer->AddInt("grid_debug", grid_debug);
	renderer->AddInt("current_scale", current_scale);
	renderer->AddInt("depthTest", depthTest);
	renderer->AddInt("wireframe", wireframe);
	renderer->AddInt("vsync", vsync);
	renderer->AddInt("frustumMainCamera", useMainCameraFrustum);
	renderer->AddInt("skybox", skybox->enabled);
	renderer->AddFloat("gammaCorrector", gammaCorrector);
	renderer->AddFloat("exposure", exposure);

	config->AddValue("renderer", *renderer);
}

void ModuleRender::Draw(const ComponentCamera &cam, int width, int height, bool isEditor) const
{
	BROFILER_CATEGORY("Render_Draw()", Profiler::Color::AliceBlue);
#ifdef GAME_BUILD
	glBindFramebuffer(GL_FRAMEBUFFER, postprocessFBO);
#endif //  GAME_BUILD
	
	glClearColor(0.3f, 0.3f, 0.3f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	SetProjectionUniform(cam);
	SetViewUniform(cam);

	if (isEditor)
	{
		glViewport(0, 0, viewScene->current_width, viewScene->current_height);
		DrawGizmos(cam);
		skybox->Draw(*cam.frustum, true);
	}
	else 
	{
#ifdef GAME_BUILD
		glViewport(0, 0, App->window->width, App->window->height);
#else
		glViewport(0, 0, viewGame->current_width, viewGame->current_height);
#endif
		skybox->Draw(*cam.frustum);
		const float transparent[] = { 0, 0, 0, 1 };
		glClearBufferfv(GL_COLOR, 1, transparent);
		glClearBufferfv(GL_COLOR, 2, transparent);
	}
	App->scene->Draw(*cam.frustum, isEditor);

	if (!isEditor)
	{
		glDepthMask(GL_FALSE);

		GLint drawFboId = 0;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);

		bool horizontal = true, first_iteration = true;
		
		glUseProgram(blur->id[0]);
		
		for (unsigned int i = 0; i < BLOOM_AMOUNT; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
			glUniform1i(glGetUniformLocation(blur->id[0], "horizontal"), horizontal);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, first_iteration ? brightnessBufferGame : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
			
			glBindVertexArray(postprocessVAO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, postprocessEBO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			horizontal = !horizontal;
			if (first_iteration)
				first_iteration = false;
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		
		glBindVertexArray(0);
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0); //Flush textures

		glBindFramebuffer(GL_FRAMEBUFFER, drawFboId);
		
		unsigned variation = 0u;
		if (cam.fogEnabled)
			variation |= (unsigned)ModuleProgram::Postprocess_Variations::FOG;

		glUseProgram(postProcessShader->id[variation]);
		glBindVertexArray(postprocessVAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, postprocessEBO);
	
		glUniform1i(glGetUniformLocation(postProcessShader->id[variation], "gColor"), 0);
		glUniform1i(glGetUniformLocation(postProcessShader->id[variation], "gHighlight"), 1);
		glUniform1i(glGetUniformLocation(postProcessShader->id[variation], "gBrightness"), 2);
		glUniform1i(glGetUniformLocation(postProcessShader->id[variation], "gDepth"), 3);
		glUniform1f(glGetUniformLocation(postProcessShader->id[variation], "gammaCorrector"), gammaCorrector);
		glUniform1f(glGetUniformLocation(postProcessShader->id[variation], "exposure"), exposure);
		
		glUniform1f(glGetUniformLocation(postProcessShader->id[variation], "fogFalloff"), 1.f / cam.fogFalloff);
		glUniform1f(glGetUniformLocation(postProcessShader->id[variation], "fogQuadratic"), cam.fogQuadratic);
		glUniform1f(glGetUniformLocation(postProcessShader->id[variation], "maxFog"), cam.maxFog);
		glUniform3fv(glGetUniformLocation(postProcessShader->id[variation], "fogColor"), 1, (GLfloat*)&cam.fogColor);		
		
		glUniform1f(glGetUniformLocation(postProcessShader->id[variation], "zNear"), cam.frustum->nearPlaneDistance);
		glUniform1f(glGetUniformLocation(postProcessShader->id[variation], "zFar"), cam.frustum->farPlaneDistance);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, renderedSceneGame);
		
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, highlightBufferGame);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
		
		glActiveTexture(GL_TEXTURE3);
#ifndef GAME_BUILD
		glBindTexture(GL_TEXTURE_2D, viewGame->depthTexture);
#else
		glBindTexture(GL_TEXTURE_2D, depthTexture);
#endif

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		
		glBindTexture(GL_TEXTURE_2D, 0);
		glUseProgram(0);
		
		glDepthMask(GL_TRUE);
		glActiveTexture(GL_TEXTURE0); //LOL without this the skybox doesn't render

		glBindFramebuffer(GL_FRAMEBUFFER, 0); //Flush textures

		glBindFramebuffer(GL_FRAMEBUFFER, drawFboId);
	}
	else
	{
		App->navigation->renderNavMesh();
	}

	if (!isEditor || isEditor && App->ui->showUIinSceneViewport)
	{
		glClear(GL_DEPTH_BUFFER_BIT);
		App->ui->Draw(width, height);		
	}

#ifdef GAME_BUILD
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, postprocessFBO);
	glDrawBuffer(GL_BACK);
	glBlitFramebuffer(0, 0, width, height, 0, 0, App->window->width, App->window->height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif // GAME_BUILD
}

bool ModuleRender::IsSceneViewFocused() const
{
	return viewScene->focus;
}

bool ModuleRender::IsSceneHovered() const
{
	return viewScene->hover;
}
// Called before quitting
bool ModuleRender::CleanUp()
{
	LOG("Destroying renderer");
	if (UBO != 0)
	{
		glDeleteBuffers(1, &UBO);
	}
	glDeleteTextures(1, &highlightBufferGame);
	glDeleteTextures(1, &brightnessBufferGame);
	glDeleteTextures(1, &renderedSceneGame);
	return true;	  
}	

void ModuleRender::OnResize()
{
#ifndef GAME_BUILD	
	App->camera->editorcamera->SetAspect((float)viewScene->current_width / (float)viewScene->current_height);
	if (App->scene->maincamera != nullptr/* && viewGame->current_width != 0 && viewGame->current_height!=0*/)
	{
		App->scene->maincamera->SetAspect((float)viewGame->current_width / (float)viewGame->current_height);
	}
#else
	glViewport(0, 0, App->window->width, App->window->height);
	if (App->scene->maincamera != nullptr)
	{
		App->scene->maincamera->SetAspect((float)App->window->width / (float)App->window->height);
	}
	CreatePostProcessFramebuffer();
#endif

	GLint drawFboId = 0;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);

	if (App->scene->maincamera != nullptr)
	{
#ifndef GAME_BUILD

		glBindFramebuffer(GL_FRAMEBUFFER, viewGame->FBO);
#else
		glBindFramebuffer(GL_FRAMEBUFFER, postprocessFBO);
#endif

		glBindTexture(GL_TEXTURE_2D, renderedSceneGame);
#ifndef GAME_BUILD
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, viewGame->current_width, viewGame->current_height, 0, GL_RGBA, GL_FLOAT, NULL);
#else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, App->window->width, App->window->height, 0, GL_RGBA, GL_FLOAT, NULL);
#endif		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		glBindTexture(GL_TEXTURE_2D, highlightBufferGame);
#ifndef GAME_BUILD
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, viewGame->current_width, viewGame->current_height, 0, GL_RGBA, GL_FLOAT, NULL);
#else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, App->window->width, App->window->height, 0, GL_RGBA, GL_FLOAT, NULL);
#endif		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		glBindTexture(GL_TEXTURE_2D, brightnessBufferGame);
#ifndef GAME_BUILD
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, viewGame->current_width, viewGame->current_height, 0, GL_RGBA, GL_FLOAT, NULL);
#else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, App->window->width, App->window->height, 0, GL_RGBA, GL_FLOAT, NULL);
#endif
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

#ifdef GAME_BUILD
		glBindTexture(GL_TEXTURE_2D, depthTexture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, App->window->width, App->window->height, 0, GL_DEPTH_COMPONENT, GL_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

#endif		

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, highlightBufferGame, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, brightnessBufferGame, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedSceneGame, 0);

		unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };// , GL_COLOR_ATTACHMENT2	};// , , GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4
		glDrawBuffers(3, attachments);

		for (unsigned int i = 0; i < 2; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
			glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
#ifndef GAME_BUILD
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, viewGame->current_width, viewGame->current_height, 0, GL_RGB, GL_FLOAT, NULL);
#else
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, App->window->width, App->window->height, 0, GL_RGB, GL_FLOAT, NULL);
#endif
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);						
		}


	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, drawFboId);
}

void ModuleRender::SetVsync(bool active)
{
	vsync = active;
	SDL_GL_SetSwapInterval((int)vsync);
}

void ModuleRender::DrawGizmos(const ComponentCamera &camera) const
{
	BROFILER_CATEGORY("Render_DrawGizmos()", Profiler::Color::AliceBlue);
	unsigned shader = App->program->defaultShader->id[0];
	glUseProgram(shader);

	if (picker_debug)
	{
		for (const auto & line : App->scene->debuglines)
		{
			dd::line(line.a, line.b, math::float3(0.0f, 0.0f, 1.0f));
		}
	}

	math::float4x4 model = math::float4x4::identity;
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_TRUE, &model[0][0]);

	if (grid_debug)
	{
		dd::xzSquareGrid(-500.0f * current_scale, 500.0f * current_scale, 0.0f, 1.0f * current_scale, math::float3(0.65f, 0.65f, 0.65f));
	}

	dd::axisTriad(math::float4x4::identity, 0.5f * current_scale, 5.0f * current_scale, 0, true);

	if (App->scene->maincamera != nullptr && App->renderer->useMainCameraFrustum)
	{
		dd::frustum((App->scene->maincamera->frustum->ProjectionMatrix() * App->scene->maincamera->frustum->ViewMatrix()).Inverted(), dd::colors::Red);
	}
	if (App->scene->selected != nullptr)
	{
		ComponentCamera* selectedCam = App->scene->selected->GetComponent<ComponentCamera>();
		if (selectedCam != nullptr)
		{
			dd::frustum((selectedCam->frustum->ProjectionMatrix() * selectedCam->frustum->ViewMatrix()).Inverted(), dd::colors::HotPink);
		}
	}
	App->debug->Draw(camera, viewScene->current_width, viewScene->current_height);
	glUseProgram(0);
}

void ModuleRender::InitSDL()
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	context = SDL_GL_CreateContext(App->window->window);
	SDL_GetWindowSize(App->window->window, &App->window->width, &App->window->height);
}

void ModuleRender::InitOpenGL() const
{
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_TEXTURE_2D);

	glClearDepth(1.0f);
	glClearColor(0.3f, 0.3f, 0.3f, 1.f);

	glViewport(0, 0, App->window->width, App->window->height);
}

Viewport* ModuleRender::GetActiveViewport() const
{
	if (!viewGame->hidden)
	{
		return viewGame;
	}
	return viewScene;
}

void ModuleRender::ComputeShadows()
{
	shadowVolumeRendered = false;
	if (directionalLight && directionalLight->produceShadows)
	{
		shadowVolumeRendered = true;
		math::AABB lightAABB;
		lightAABB.SetNegativeInfinity();
		bool renderersDetected = false;
		shadowCasters.clear();
		//TODO: Improve this avoiding shuffle every frame
		for (GameObject* go : App->scene->dynamicFilteredGOs) 
		{
			ComponentRenderer* cr = go->GetComponent<ComponentRenderer>();
			if (cr != nullptr && cr->castShadows)
			{
				renderersDetected = true;
				lightAABB.Enclose(go->bbox);
				shadowCasters.insert(cr);
			}
		}

		for (GameObject* go : App->scene->staticFilteredGOs)
		{
			ComponentRenderer* cr = (ComponentRenderer*)go->GetComponentOld(ComponentType::Renderer);
			if (cr && cr->castShadows)
			{
				renderersDetected = true;
				lightAABB.Enclose(go->bbox);
			}
		}
		if (!renderersDetected)
		{
			return;
		}
		//TODO: End improving zone
		
		math::float3 points[8];

		math::float4x4 lightMat = math::Quat::LookAt(math::float3::unitZ, directionalLight->gameobject->transform->front, math::float3::unitY, directionalLight->gameobject->transform->up).Inverted().ToFloat3x3();

		lightAABB.SetFromCenterAndSize(lightAABB.CenterPoint(), lightAABB.Size() * 1.5f);
		lightAABB.GetCornerPoints(points);
		math::float3 minP, maxP;
		minP = points[0];
		maxP = points[0];
		for (unsigned i = 0u; i < 8u; ++i)
		{
			points[i] = lightMat.TransformPos(points[i]);
			minP.x = MIN(points[i].x, minP.x);
			minP.y = MIN(points[i].y, minP.y);
			minP.z = MIN(points[i].z, minP.z);

			maxP.x = MAX(points[i].x, maxP.x);
			maxP.y = MAX(points[i].y, maxP.y);
			maxP.z = MAX(points[i].z, maxP.z);
		}

		shadowVolumeWidth = maxP.x - minP.x;
		shadowVolumeWidthHalf = shadowVolumeWidth * .5f;
		shadowVolumeHeight = maxP.y - minP.y;
		shadowVolumeHeightHalf = shadowVolumeHeight * .5f;
		shadowVolumeLength = maxP.z - minP.z;

		lightPos = lightMat.Inverted().TransformPos(math::float3((maxP.x + minP.x) * .5f, (maxP.y + minP.y) * .5f, maxP.z));

		if (shadowDebug) // draw shadows volume
		{
			ShadowVolumeDrawDebug();
		}
		shadowsFrustum.up = -directionalLight->gameobject->transform->up;
		shadowsFrustum.front = -directionalLight->gameobject->transform->front;
		BlitShadowTexture();
	}
}

void ModuleRender::ShadowVolumeDrawDebug() const
{
	dd::sphere(lightPos, dd::colors::YellowGreen, current_scale);

	math::float3 lFront = directionalLight->gameobject->transform->front;
	math::float3 lRight = directionalLight->gameobject->transform->right;
	math::float3 lUp = directionalLight->gameobject->transform->up;


	dd::line(lightPos, lightPos - lFront * shadowVolumeLength, dd::colors::YellowGreen);
	dd::line(lightPos, lightPos - lRight * shadowVolumeWidthHalf, dd::colors::Red);
	dd::line(lightPos, lightPos + lRight * shadowVolumeWidthHalf, dd::colors::Red);
	dd::line(lightPos, lightPos - lUp * shadowVolumeHeightHalf, dd::colors::Green);
	dd::line(lightPos, lightPos + lUp * shadowVolumeHeightHalf, dd::colors::Green);

	math::float3 nearCorners[4] = {
				lightPos + lUp * shadowVolumeHeightHalf + lRight * shadowVolumeWidthHalf,
				lightPos + lUp * shadowVolumeHeightHalf - lRight * shadowVolumeWidthHalf,
				lightPos - lUp * shadowVolumeHeightHalf - lRight * shadowVolumeWidthHalf,
				lightPos - lUp * shadowVolumeHeightHalf + lRight * shadowVolumeWidthHalf
	};

	math::float3 farCorners[4] = {
				lightPos - lFront * shadowVolumeLength + lUp * shadowVolumeHeightHalf + lRight * shadowVolumeWidthHalf,
				lightPos - lFront * shadowVolumeLength + lUp * shadowVolumeHeightHalf - lRight * shadowVolumeWidthHalf,
				lightPos - lFront * shadowVolumeLength - lUp * shadowVolumeHeightHalf - lRight * shadowVolumeWidthHalf,
				lightPos - lFront * shadowVolumeLength - lUp * shadowVolumeHeightHalf + lRight * shadowVolumeWidthHalf
	};

	for (unsigned i = 0u; i < 4u; ++i)
	{
		dd::line(nearCorners[i], farCorners[i], dd::colors::YellowGreen);
		if (i < 3u)
		{
			dd::line(nearCorners[i], nearCorners[i + 1], dd::colors::YellowGreen);
			dd::line(farCorners[i], farCorners[i + 1], dd::colors::YellowGreen);
		}
		else
		{
			dd::line(nearCorners[i], nearCorners[0], dd::colors::YellowGreen);
			dd::line(farCorners[i], farCorners[0], dd::colors::YellowGreen);
		}
	}
}

void ModuleRender::BlitShadowTexture()
{
	glBindFramebuffer(GL_FRAMEBUFFER, shadowsFBO);
	glBindTexture(GL_TEXTURE_2D, shadowsTex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, shadowVolumeWidth, shadowVolumeHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowsTex, 0);
	glDrawBuffer(GL_NONE);
	glViewport(0, 0, shadowVolumeWidth, shadowVolumeHeight);
	glClear(GL_DEPTH_BUFFER_BIT);
	shadowsFrustum.orthographicHeight = shadowVolumeHeight;
	shadowsFrustum.orthographicWidth = shadowVolumeWidth;
	shadowsFrustum.nearPlaneDistance = .0f;
	shadowsFrustum.farPlaneDistance = shadowVolumeLength;
	shadowsFrustum.pos = lightPos;
	
		
	for (ComponentRenderer* cr : shadowCasters)
	{
		unsigned variation = 0u;
		if (cr->mesh)
		{
			if (cr->mesh->bindBones.size() > 0u)
			{
				variation |= (unsigned)ModuleProgram::Shadows_Variations::SKINNED;
			}
			glUseProgram(shadowsShader->id[variation]);
			glUniformMatrix4fv(glGetUniformLocation(shadowsShader->id[variation],
				"viewProjection"), 1, GL_TRUE, &shadowsFrustum.ViewProjMatrix()[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(shadowsShader->id[variation],
				"model"), 1, GL_TRUE, &cr->gameobject->GetGlobalTransform()[0][0]);
			cr->DrawMesh(shadowsShader->id[variation]);
		}
	}

	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

}

void ModuleRender::CreatePostProcessFramebuffer()
{
	if (postprocessFBO == 0)
	{
		glGenFramebuffers(1, &postprocessFBO);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, postprocessFBO);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		LOG("Framebuffer ERROR");
}

void ModuleRender::DrawGUI()
{
	if (ImGui::Checkbox("Depth Test", &depthTest))
	{
		if (App->renderer->depthTest)
		{
			glEnable(GL_DEPTH_TEST);
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
		}
	}

	if (ImGui::Checkbox("Wireframe", &wireframe))
	{
		if (wireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}
	if (ImGui::Checkbox("Vsync", &vsync))
	{
		SDL_GL_SetSwapInterval((int)vsync);
	}
	ImGui::Checkbox("Game Frustum", &useMainCameraFrustum);
	ImGui::Checkbox("Skybox", &skybox->enabled);
	ImGui::Checkbox("MSAA", &msaa);
	if (msaa)
	{
		int previous_msaa_level = msaa_level;
		ImGui::RadioButton("x2", &msaa_level, 2); ImGui::SameLine();
		ImGui::RadioButton("x4", &msaa_level, 4); ImGui::SameLine();
		ImGui::RadioButton("x8", &msaa_level, 8);
		if (previous_msaa_level != msaa_level)
		{
			msaa_lvl_changed = true;
		}
	}
	ImGui::Checkbox("Picker Debug", &picker_debug);
	ImGui::Checkbox("Light Debug", &light_debug);
	ImGui::Checkbox("Shadow volume Debug", &shadowDebug);
	if (shadowDebug && App->renderer->shadowVolumeRendered)
	{
		ImGui::Image((ImTextureID)App->renderer->shadowsTex, { 200,200 }, { 0,1 }, { 1,0 });
	}

	ImGui::Checkbox("Dynamic AABBTree Debug", &aabbTreeDebug);
	ImGui::Checkbox("Static KDTree Debug", &kDTreeDebug);
	ImGui::Checkbox("Grid Debug", &grid_debug);
	ImGui::Checkbox("Bone Debug", &boneDebug);
	ImGui::Checkbox("Pathfinding debug", &pathfindingDebug);

	const char* scales[] = { "1", "10", "100" };
	ImGui::Combo("Scale", &item_current, scales, 3);
	unsigned new_scale = atoi(scales[item_current]);
	if (new_scale != current_scale)
	{
		current_scale = new_scale;
		std::vector<Component*> cameras = App->scene->root->GetComponentsInChildren(ComponentType::Camera);
		for (auto &cam : cameras)
		{
			((ComponentCamera*)cam)->frustum->nearPlaneDistance = ZNEARDIST * current_scale;
			((ComponentCamera*)cam)->frustum->farPlaneDistance = ZFARDIST * current_scale;
		}
		App->camera->editorcamera->frustum->nearPlaneDistance = ZNEARDIST * current_scale;
		App->camera->editorcamera->frustum->farPlaneDistance = ZFARDIST * current_scale;
	}
	ImGui::DragFloat("Gamma correction", &gammaCorrector, .05f, 1.2f, 3.2f);
	ImGui::DragFloat("Exposure", &exposure, .05f, .1f, 10.0f);

}

void ModuleRender::GenBlockUniforms()
{
	glGenBuffers(1, &UBO); //Block uniform creation
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	//TODOALF: this should be nullptr instead unll
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(float4x4), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferRange(GL_UNIFORM_BUFFER, 0, UBO, 0, 2 * sizeof(float4x4));
}

void ModuleRender::AddBlockUniforms(const Shader &shader) const
{
	for (auto id : shader.id)
	{
		unsigned int uniformBlockIndex = glGetUniformBlockIndex(id.second, "Matrices");
		glUniformBlockBinding(id.second, uniformBlockIndex, 0);
	}
}

void ModuleRender::SetViewUniform(const ComponentCamera &camera) const
{
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float4x4), sizeof(float4x4), &camera.GetViewMatrix()[0][0]);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void ModuleRender::SetProjectionUniform(const ComponentCamera &camera) const
{
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float4x4), &camera.GetProjectionMatrix()[0][0]);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
