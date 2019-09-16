#include "Application.h"
#include "Globals.h"

#include "ModuleResourceManager.h"
#include "ModuleFileSystem.h"
#include "ModuleEditor.h"
#include "ModuleScene.h"
#include "ModuleTextures.h"
#include "ModuleParticles.h"

#include "GameObject.h"
#include "ComponentParticles.h"
#include "ResourceTexture.h"

#include "HashString.h"
#include "imgui.h"
#include "JSON.h"
#include "debugdraw.h"
#include "imgui_color_gradient.h"
#include "Brofiler.h"
#include "Algorithm/Random/LCG.h"

#define None "None Selected"

ComponentParticles::ComponentParticles(GameObject* gameobject) : Component(gameobject, ComponentType::Particles)
{
	textureFiles = App->resManager->GetResourceNamesList(TYPE::TEXTURE, true);
	App->particles->AddParticleSystem(this);
	modules.push_back(new PMSizeOverTime());
	modules.push_back(new PMColorOverTime());
}

ComponentParticles::ComponentParticles(const ComponentParticles& component) : Component(component)
{
	textureFiles = App->resManager->GetResourceNamesList(TYPE::TEXTURE, true);
	texture = (ResourceTexture*)App->resManager->Get(component.texture->GetUID());
	xTiles = component.xTiles;
	yTiles = component.yTiles;
	fps = component.fps;
	timer = component.timer;
	frameMix = component.frameMix;
	f1Xpos = component.f1Xpos;
	f1Ypos = component.f1Ypos;
	f2Xpos = component.f2Xpos;
	f2Ypos = component.f2Ypos;

	lifetime = component.lifetime;
	speed = component.speed;
	rate = component.rate;
	rateTimer = 1.f / rate;
	maxParticles = component.maxParticles;
	axisRotation = component.axisRotation;
	particleMinSize = component.particleMinSize;
	particleMaxSize = component.particleMaxSize;
	quadEmitterSize = component.quadEmitterSize;
	particleColor = component.particleColor;
	directionNoise = component.directionNoise;
	directionNoiseProbability = component.directionNoiseProbability;
	directionNoiseTotalProbability = component.directionNoiseTotalProbability;
	actualEmisor = component.actualEmisor;
	alternateEmisor(actualEmisor);

	App->particles->AddParticleSystem(this);
	modules.push_back(new PMSizeOverTime(*(PMSizeOverTime*)component.modules[0]));
	modules.push_back(new PMColorOverTime(*(PMColorOverTime*)component.modules[1]));
}

ComponentParticles::~ComponentParticles()
{
	if (texture != nullptr)
	{
		App->resManager->DeleteResource(texture->GetUID());
		texture = nullptr;
	}
}

void ComponentParticles::Reset()
{
	unsigned nParticles = particles.size();

	for (; nParticles > 0; --nParticles)
	{
		RELEASE(particles.front());
		particles.pop_front();
	}
}

Component* ComponentParticles::Clone() const
{
	return new ComponentParticles(*this);
}

void ComponentParticles::DrawProperties()
{
	ImGui::PushID(this);

	if (ImGui::CollapsingHeader("Particle System")) 
	{
		bool removed = Component::DrawComponentState();
		if (removed)
		{
			ImGui::PopID();
			return;
		}

		ImGui::Text("Particles active %d", particles.size());
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

			if (textureFiles.empty())
			{
				textureFiles = App->resManager->GetResourceNamesList(TYPE::TEXTURE, true);
			}

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
		if (texture != nullptr)
		{
			ImGui::Image((ImTextureID)texture->gpuID, { 200,200 }, { 0,1 }, { 1,0 });
		}

		ImGui::InputInt("X Tiles", &xTiles);
		ImGui::InputInt("Y Tiles", &yTiles);
		xTiles = MAX(xTiles, 1);
		yTiles = MAX(yTiles, 1);
		if (ImGui::InputFloat("FPS", &fps, 1.f))
		{
			timer = 0.f;
		}
		ImGui::Separator();
		ImGui::Checkbox("Direction noise", &directionNoise);
		if (directionNoise)
		{
			ImGui::InputInt("Probability", &directionNoiseProbability);
			ImGui::InputInt("Total Probability", &directionNoiseTotalProbability);
			ImGui::Separator();
		}
		ImGui::InputInt("Max Particles", &maxParticles);
		ImGui::DragFloat("Rate", &rate, 0.1f, 0.1f, MAX_RATE);

		ImGui::Text("Particle properties:");
		ImGui::Checkbox("Billboarded", &billboarded);
		if (!billboarded)
		{
			ImGui::Checkbox("Aligned", &aligned);
		}
		ImGui::Checkbox("Local Emitter", &localEmitter);
		if (!billboarded)
		{
			ImGui::InputFloat3("LookAt vector(local X,Y,Z)", &lookAtTarget[0]);
		}
		if (billboarded)
		{
			ImGui::DragFloat2("Axis Rotation", (float*)&axisRotation, 1.0f, 0.0f, 360.f);
		}
		ImGui::DragFloat2("Lifetime", &lifetime[0], 0.1f);


		ImGui::DragFloat2("Speed", &speed[0], 1.2f);
		ImGui::DragFloat2("MinSize(W,H)", &particleMinSize[0], 0.01 * App->renderer->current_scale);
		ImGui::DragFloat2("MaxSize(W,H)", &particleMaxSize[0], 0.01 * App->renderer->current_scale);
		ImGui::ColorEdit3("Color", (float*)&particleColor);
		ImGui::DragFloat("Intensity", &intensity, 0.05f, 0.01f, 10.0f);

		//Clamp values
		lifetime[0] = Max(0.01f, lifetime[0]);
		lifetime[1] = Max(lifetime[0], lifetime[1]);

		/*particleSize[0] = Max(0.01f, particleSize[0]);
		particleSize[1] = Max(particleSize[0], particleSize[1]);
		*/
		//
		ImGui::Text("Emisor type:");
		if (ImGui::Checkbox("Quad", &quadCheck))    alternateEmisor(0);
		if (ImGui::Checkbox("Sphere", &sphereCheck)) alternateEmisor(1);
		if (ImGui::Checkbox("Cone", &coneCheck)) alternateEmisor(2);

		switch (actualEmisor)
		{
		case EmisorType::QUAD:
			ImGui::DragFloat2("Quad size", &quadEmitterSize[0], 0.1 * App->renderer->current_scale);
			break;
		case EmisorType::CONE:
			ImGui::DragFloat("Base Radius", &baseRadius, 0.1f, 0.0f, 5.0f);
			ImGui::DragFloat("Apex Radius", &apexRadius, 0.1f, 0.0f, 5.0f);
			break;
		}

		for (ParticleModule* pm : modules)
		{
			ImGui::Separator();
			pm->InspectorDraw();
		}
	
	}
	ImGui::PopID();

	if (gameobject->isSelected) DrawDebugEmisor();

}

bool ComponentParticles::CleanUp()
{
	unsigned nParticles = particles.size();

	for (; nParticles > 0; --nParticles)
	{
		RELEASE(particles.front());
		particles.pop_front();
	}

	nParticles = particlePool.size();

	for (; nParticles > 0; --nParticles)
	{
		RELEASE(particlePool.front());
		particlePool.pop();
	}

	App->particles->RemoveParticleSystem(this);
	return true;
}

void ComponentParticles::Update(float dt, const math::float3& camPos)
{
	BROFILER_CATEGORY("Update ParticleSystem", Profiler::Color::AliceBlue);
	if (gameobject->particlesDirty)
	{
		while (!particles.empty())
		{
			RELEASE(particles.front());
			particles.pop_back();
			rateTimer = 0.f;
		}
		gameobject->particlesDirty = false;
	}

	timer += dt; 

	float currentFrame = timer / (1 / fps);
	float frame;
	frameMix = modf(currentFrame, &frame);
	math::float3 pos;
	if (!localEmitter)
	{
		pos = gameobject->transform->GetGlobalPosition();
	}
	else
	{
		pos = math::float3::zero;
	}
	f1Xpos = ((int)frame) % xTiles;
	f2Xpos = (f1Xpos + 1) % xTiles;
	f1Ypos = (((int)frame) / xTiles) % yTiles;
	f2Ypos = f1Xpos < f2Xpos ? f1Ypos : f1Ypos + 1;	

	rateTimer -= dt;

	//Update the active modules with local checks
	sizeOTCheck = modules[0]->enabled;
	colorOTCheck = modules[1]->enabled;

	//Create new Particle P
	if (gameobject->isActive() && rateTimer <= 0.f && particles.size() < maxParticles)
	{
		int amount = MIN(MAX(dt / (1.f / rate), 1), maxParticles);
		for (; amount > 0; --amount)
		{
			Particle* p;
			if (particlePool.size() > 0)
			{
				p = particlePool.front();
				particlePool.pop();
			}
			else
			{
				p = new Particle();				
			}

			//P direction
			if (directionNoise)
			{
				float xD = ((rand() % 100) - 50) / 100.f;
				float yD = ((rand() % 100) - 50) / 100.f;
				float zD = ((rand() % 100) - 50) / 100.f;
				p->direction = math::float3(xD, yD, zD);
				p->direction.Normalize();
			}

			LCG mathRand;
			switch (actualEmisor)
			{
			case EmisorType::QUAD:
			{
				quadEmitterSize.x = MAX(1, quadEmitterSize.x);
				quadEmitterSize.y = MAX(1, quadEmitterSize.y);

				//P starting position
				float3x3 rot = gameobject->transform->global.RotatePart();
				p->position = pos + rot * float3(rand() % (int)quadEmitterSize.x - quadEmitterSize.x *.5f, 0, rand() % (int)quadEmitterSize.y - quadEmitterSize.y *.5f);

				//P direction
				p->direction = gameobject->transform->up;// *float3(0.f, 1.f, 0.f); //math::float3::unitY;

				//P starting rotation
				p->axisRotation = math::DegToRad(mathRand.Float(axisRotation.x, axisRotation.y));
				break;
			}
			case EmisorType::SPHERE:
				//P starting position
				p->position = pos;
				//P direction
				p->direction = (randomSpherePoint(pos) - pos).Normalized();

				//P starting rotation
				p->axisRotation = math::DegToRad(mathRand.Float(axisRotation.x, axisRotation.y));
				break;

			case EmisorType::CONE:
				math::Circle apexCircle(pos, gameobject->transform->up, apexRadius*App->renderer->current_scale);

				//P starting position
				math::float3 pointCircle = apexCircle.RandomPointInside(mathRand);
				float percentage = mathRand.Float();
				p->position = math::float3::Lerp(pos, pointCircle, percentage);

				//P direction
				math::Circle baseCircle(pos + gameobject->transform->up * speed.y * lifetime.y, gameobject->transform->up, baseRadius * App->renderer->current_scale);
				
				float3 apexDirection = (pointCircle - apexCircle.pos).Normalized();
				float3 pointBaseCircle = baseCircle.ExtremePoint(apexDirection);
				float3 insideBasePoint = math::float3::Lerp(baseCircle.pos, pointBaseCircle, percentage);
				
				p->direction = (insideBasePoint - p->position).Normalized();

				//P starting rotation
				p->axisRotation = math::DegToRad(mathRand.Float(axisRotation.x, axisRotation.y));
				break;
			}
		
			//P lifetime
			if (lifetime.x != lifetime.y)
			{
				p->totalLifetime = lifetime.x + fmod(rand(), abs(lifetime.y - lifetime.x));
			}
			else
			{
				p->totalLifetime = lifetime.y;
			}

			//P speed
			if (speed.x != speed.y)
			{
				p->speed = speed.x + fmod(rand(), abs(speed.y - speed.x));
			}
			else
			{
				p->speed = speed.y;
			}
			p->lifeTimer = p->totalLifetime;

			// P color
			p->color = particleColor;
			// P size
			float percentage = mathRand.Float();
			p->size = particleMinSize + (particleMaxSize - particleMinSize) * percentage;
			particles.push_back(p);
		}
		rateTimer = 1.f / rate;

	}


	unsigned nParticles = particles.size();
	
	for (; nParticles > 0; --nParticles)
	{
		if (directionNoise && (rand() % directionNoiseTotalProbability) < directionNoiseProbability)
		{			
			float xD = ((rand() % 100) - 50) / 100.f;
			float yD = ((rand() % 100) - 50) / 100.f;
			float zD = ((rand() % 100) - 50) / 100.f;
			particles.front()->direction += math::float3(xD, yD, zD);
			particles.front()->direction.Normalize();
		}
		float sizeOT = 1.0f;
		float4 newColor;

		for (ParticleModule* pm : modules)
		{
			
			float currentTimeOverTotal = particles.front()->lifeTimer / particles.front()->totalLifetime;

			switch (pm->type)
			{
			case ParticleModule::ParticleModulesType::SIZE_OVER_TIME:
				if (pm->enabled) sizeOT = ((PMSizeOverTime*)pm)->GetSize(currentTimeOverTotal, 1.0f);
				break;
			case ParticleModule::ParticleModulesType::COLOR_OVER_TIME:

				if (pm->enabled) ((PMColorOverTime*)pm)->Imgradient->getColorAt(1.f - currentTimeOverTotal, newColor);
				else newColor = particleColor;
				break;
			}
			
			
		}
		particles.front()->direction.Normalize();
		particles.front()->position += particles.front()->direction * particles.front()->speed * dt;
		

		if (billboarded)
		{
			if (localEmitter)
			{
				pos = particles.front()->position + gameobject->transform->GetGlobalPosition();
				float3 direction = (camPos - pos);
				particles.front()->global = particles.front()->global.FromTRS(pos, math::Quat::LookAt(float3::unitZ, direction.Normalized(), 
					float3::unitY, float3::unitY).Mul(Quat::FromEulerXYZ(0, 0, particles.front()->axisRotation)), 
					math::float3(particles.front()->size.x, particles.front()->size.y, 1.0f) * sizeOT);
			}
			else
			{
				float3 direction = (camPos - particles.front()->position);
				particles.front()->global = particles.front()->global.FromTRS(particles.front()->position, 
					math::Quat::LookAt(float3::unitZ, direction.Normalized(), float3::unitY, float3::unitY)
					.Mul(Quat::FromEulerXYZ(0, 0, particles.front()->axisRotation)),
					math::float3(particles.front()->size.x, particles.front()->size.y, 1.0f) * sizeOT);
			}
		}
		else
		{
			math::Quat rot;
			if (aligned)
			{
				rot = gameobject->transform->GetRotation();
			}
			else
			{
				rot = math::Quat::LookAt(float3::unitZ, lookAtTarget.Normalized(), float3::unitY, float3::unitY);
			}
			if (localEmitter)
			{
				particles.front()->global = particles.front()->global.FromTRS(particles.front()->position + gameobject->transform->GetGlobalPosition(), 
					rot, math::float3(particles.front()->size.x, particles.front()->size.y, 1.0f) * sizeOT);
			}
			else
			{
				particles.front()->global = particles.front()->global.FromTRS(particles.front()->position, rot, 
					math::float3(particles.front()->size.x, particles.front()->size.y, 1.0f) * sizeOT);
			}
		}
		particles.front()->color = newColor;

		particles.push_back(particles.front());
		particles.pop_front();
	}
}

void ComponentParticles::Save(JSON_value* value) const
{
	Component::Save(value);
	value->AddInt("xTiles", xTiles);
	value->AddInt("yTiles", yTiles);
	value->AddFloat("fps", fps);
	value->AddUint("textureUID", (texture != nullptr) ? texture->GetUID() : 0u);
	value->AddFloat2("lifetime", lifetime);
	value->AddFloat2("speed", speed);
	value->AddFloat("rate", rate);
	value->AddInt("maxParticles", maxParticles);
	value->AddFloat2("rotation", axisRotation);
	value->AddFloat2("sizeMin", particleMinSize);
	value->AddFloat2("sizeMax", particleMaxSize);
	value->AddFloat2("quadEmitterSize", quadEmitterSize);
	value->AddFloat4("particleColor", particleColor);
	value->AddInt("directionNoise", directionNoise);
	value->AddInt("directionNoiseProbability", directionNoiseProbability);
	value->AddInt("directionNoiseTotalProbability", directionNoiseTotalProbability);
	value->AddInt("actualEmisor", actualEmisor);
	if (actualEmisor == EmisorType::CONE)
	{
		value->AddFloat("baseRadius", baseRadius);
		value->AddFloat("apexRadius", apexRadius);
	}

	value->AddInt("sizeOT", sizeOTCheck);
	value->AddInt("colorOT", colorOTCheck);

	value->AddFloat4("defaultColor", particleColor);

	value->AddInt("billboarded", billboarded);
	value->AddInt("aligned", aligned);
	value->AddInt("localEmitter", localEmitter);
	value->AddFloat3("lookAtTarget", lookAtTarget);

	value->AddFloat("intensity", intensity);
	PMSizeOverTime* SOTAux = (PMSizeOverTime*)modules[0];
	value->AddFloat4("bezier14", float4(SOTAux->v[0], SOTAux->v[1], SOTAux->v[2], SOTAux->v[3]));
	value->AddFloat("bezier5", float(SOTAux->v[4]));

	PMColorOverTime* COTAux = (PMColorOverTime*)modules[1];
	std::list<ImGradientMark*> marks = COTAux->Imgradient->getMarks();

	int colorI = 0;
	int alphaI = 0;
	float3 colorAux;

	for (ImGradientMark* grad : marks)
	{
		if (grad->alpha)
		{
			value->AddFloat((std::string("alpha")+std::to_string(alphaI)).c_str(), grad->color[0]);
			value->AddFloat((std::string("alphaPosition")+std::to_string(alphaI)).c_str(), grad->position);			
			++alphaI;
		}
		else
		{
			colorAux.x = grad->color[0];
			colorAux.y = grad->color[1];
			colorAux.z = grad->color[2];
			value->AddFloat3((std::string("color") + std::to_string(colorI)).c_str(), colorAux);
			value->AddFloat((std::string("colorPosition") + std::to_string(colorI)).c_str(), grad->position);
			++colorI;
		}
	}
	value->AddInt("alphas", alphaI);
	value->AddInt("colors", colorI);
}

void ComponentParticles::Load(JSON_value* value)
{
	Component::Load(value);
	xTiles = value->GetInt("xTiles");
	yTiles = value->GetInt("yTiles");
	fps = value->GetFloat("fps");

	unsigned uid = value->GetUint("textureUID");
	texture = (ResourceTexture*)App->resManager->Get(uid);
  
	lifetime = value->GetFloat2("lifetime");
	speed = value->GetFloat2("speed");
	rate = value->GetFloat("rate");
	rateTimer = 1.f / rate;
	maxParticles = value->GetInt("maxParticles");
	axisRotation = value->GetFloat2("rotation");
	particleMinSize = value->GetFloat2("sizeMin");
	particleMaxSize = value->GetFloat2("sizeMax");
	if (particleMinSize == float2::zero && particleMaxSize == float2::zero) //Compatibility mode with old particle component
	{
		particleMinSize = value->GetFloat2("size");
		particleMaxSize = particleMinSize;
	}
	quadEmitterSize = value->GetFloat2("quadEmitterSize");
	particleColor = value->GetFloat4("particleColor");
	directionNoise = value->GetInt("directionNoise");
	directionNoiseProbability = value->GetInt("directionNoiseProbability");
	directionNoiseTotalProbability = MAX(value->GetInt("directionNoiseTotalProbability"), 1);
	actualEmisor = static_cast<EmisorType>(value->GetInt("actualEmisor"));
	if (actualEmisor == EmisorType::CONE)
	{
		baseRadius = value->GetFloat("baseRadius", baseRadius);
		apexRadius = value->GetFloat("apexRadius", apexRadius);
	}

	alternateEmisor(actualEmisor);
	modules[0]->enabled = value->GetInt("sizeOT");
	modules[1]->enabled = value->GetInt("colorOT");

	particleColor = value->GetFloat4("defaultColor");

	billboarded = value->GetInt("billboarded");
	localEmitter = value->GetInt("localEmitter");
	lookAtTarget = value->GetFloat3("lookAtTarget");
	intensity = value->GetFloat("intensity", intensity);
	aligned = value->GetInt("aligned");

	PMSizeOverTime* SOTAux = (PMSizeOverTime*)modules[0];
	SOTAux->v[0] = value->GetFloat4("bezier14").x;
	SOTAux->v[1] = value->GetFloat4("bezier14").y;
	SOTAux->v[2] = value->GetFloat4("bezier14").z;
	SOTAux->v[3] = value->GetFloat4("bezier14").w;
	SOTAux->v[4] = value->GetFloat("bezier5");

	PMColorOverTime* COTAux = (PMColorOverTime*)modules[1];
	int alphaI = value->GetInt("alphas");
	int colorI = value->GetInt("colors");
	COTAux->Imgradient->clearMarks();

	for (unsigned i = 0u; i < colorI; ++i)
	{
		math::float3 colorAux = value->GetFloat3((std::string("color") + std::to_string(i)).c_str());
		COTAux->Imgradient->addMark(value->GetFloat((std::string("colorPosition") + std::to_string(i)).c_str()), ImColor(colorAux.x, colorAux.y, colorAux.z));
	}

	for (unsigned i = 0u; i < alphaI; ++i)
	{
		float alphaVal = value->GetFloat((std::string("alpha") + std::to_string(i)).c_str());
		COTAux->Imgradient->addAlphaMark(value->GetFloat((std::string("alphaPosition") + std::to_string(i)).c_str()), alphaVal);
	}
}

void ComponentParticles::alternateEmisor(int newEmisor)
{
	for (int i = 0; i < emisorsCheck.size(); i++) *emisorsCheck[i] = false;
	*emisorsCheck[newEmisor] = true;
	actualEmisor = static_cast<EmisorType>(newEmisor);
}

void ComponentParticles::DrawDebugEmisor()
{
	float3 base = gameobject->transform->GetGlobalPosition();
	float3x3 rot = gameobject->transform->global.RotatePart();
	float3 v1 = base + rot * float3(quadEmitterSize.x * .5f, 0, -quadEmitterSize.y * .5f);
	float3 v2 = base + rot * float3(quadEmitterSize.x * .5f, 0, quadEmitterSize.y * .5f);
	float3 v3 = base + rot * float3(-quadEmitterSize.x *.5f, 0, quadEmitterSize.y * .5f);
	float3 v4 = base + rot * float3(-quadEmitterSize.x *.5f, 0, -quadEmitterSize.y * .5f);

	switch (actualEmisor) 
	{
	case EmisorType::QUAD:
	
		dd::line(v1, v2, dd::colors::Green);
		dd::line(v2, v3, dd::colors::Green);
		dd::line(v3, v4, dd::colors::Green);
		dd::line(v4, v1, dd::colors::Green);
		dd::line(gameobject->transform->GetGlobalPosition(), gameobject->transform->GetGlobalPosition() + gameobject->transform->up * App->renderer->current_scale * 10.f, dd::colors::Green);
		break;

	case EmisorType::SPHERE:
		dd::sphere(base, dd::colors::Green, speed.y*lifetime.y);
		break;

	case EmisorType::CONE:
		dd::cone(gameobject->transform->GetGlobalPosition(), gameobject->transform->up *speed.y*lifetime.y, dd::colors::Green, baseRadius*App->renderer->current_scale, apexRadius* App->renderer->current_scale);
		break;
	}

}

float3 ComponentParticles::randomSpherePoint(float3 center)
{
	float u = (rand() % 100) / 100.f;
	float v = (rand() % 100) / 100.f;
	float theta = 2.f * math::pi * u;
	float phi = math::Acos(2 * v - 1);
	float x = center.x + (math::Sin(phi) * math::Cos(theta));
	float y = center.y + (math::Sin(phi) * math::Sin(theta));
	float z = center.z + (math::Cos(phi));

	return float3(x, y, z);	
}


