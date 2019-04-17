#include "ComponentTransform2D.h"
#include "Application.h"
#include "GameObject.h"
#include "ModuleTime.h"
#include "ModuleRender.h"
#include "ModuleWindow.h"
#include "Viewport.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "JSON.h"

ComponentTransform2D::ComponentTransform2D(GameObject* gameobject) : Component(gameobject, ComponentType::Transform2D)
{
	alignments[TOPLEFT] = math::float2(-1.0f, 1.0f);
	alignments[TOPCENTER] = math::float2(0.0f, 1.0f);
	alignments[TOPRIGHT] = math::float2(1.0f, 1.0f);
	alignments[MIDDLELEFT] = math::float2(-1.0f, 0.0f);
	alignments[MIDDLECENTER] = math::float2(0.0f, 0.0f);
	alignments[MIDDLERIGHT] = math::float2(1.0f, 0.0f);
	alignments[BOTTOMLEFT] = math::float2(-1.0f, -1.0f);
	alignments[BOTTOMCENTER] = math::float2(0.0f, -1.0f);
	alignments[BOTTOMRIGHT] = math::float2(1.0f, -1.0f);
}

ComponentTransform2D::ComponentTransform2D(const ComponentTransform2D & component) : Component(component)
{
	position = component.position;
	size = component.size;
	alignments[TOPLEFT] = math::float2(-1.0f, 1.0f);
	alignments[TOPCENTER] = math::float2(0.0f, 1.0f);
	alignments[TOPRIGHT] = math::float2(1.0f, 1.0f);
	alignments[MIDDLELEFT] = math::float2(-1.0f, 0.0f);
	alignments[MIDDLECENTER] = math::float2(0.0f, 0.0f);
	alignments[MIDDLERIGHT] = math::float2(1.0f, 0.0f);
	alignments[BOTTOMLEFT] = math::float2(-1.0f, -1.0f);
	alignments[BOTTOMCENTER] = math::float2(0.0f, -1.0f);
	alignments[BOTTOMRIGHT] = math::float2(1.0f, -1.0f);
	currentAnchor = component.currentAnchor;
}

ComponentTransform2D::~ComponentTransform2D()
{
}

Component * ComponentTransform2D::Clone() const
{
	return new ComponentTransform2D(*this);
}

void ComponentTransform2D::DrawProperties()
{

	if (ImGui::CollapsingHeader("Rect Transformation", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (gameobject->isStatic && App->time->gameState != GameState::RUN)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		ImGui::DragFloat2("Position", (float*)&position, 0.1f, -10000.f, 10000.f);

		ImGui::DragFloat2("Size", (float*)&size, 0.5f, 0.f, 10000.f);
	
		if (gameobject->isStatic && App->time->gameState != GameState::RUN)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}


		if (ImGui::CollapsingHeader("Anchor", ImGuiTreeNodeFlags_DefaultOpen))
		{
			//top
			if (ImGui::Button("Top Left")) currentAnchor = TOPLEFT;
			
			ImGui::SameLine();
			if (ImGui::Button("Top Center")) currentAnchor = TOPCENTER;

			ImGui::SameLine();
			if (ImGui::Button("Top Right")) currentAnchor = TOPRIGHT;

			//middle
			if (ImGui::Button("Middle Left")) currentAnchor = MIDDLELEFT;

			ImGui::SameLine();
			if (ImGui::Button("Middle Center"))	currentAnchor = MIDDLECENTER;

			ImGui::SameLine();
			if (ImGui::Button("Middle Right")) currentAnchor = MIDDLERIGHT;

			//bottom
			if (ImGui::Button("Bottom Left")) currentAnchor = BOTTOMLEFT;

			ImGui::SameLine();
			if (ImGui::Button("Bottom Center"))	currentAnchor = BOTTOMCENTER;

			ImGui::SameLine();
			if (ImGui::Button("Bottom Right")) currentAnchor = BOTTOMRIGHT;
		}
	}
}

bool ComponentTransform2D::CleanUp()
{
	alignments.clear();//clears array
	alignments.shrink_to_fit();//reduces the capacity to fit its size, which is 0
	return true;
}

void ComponentTransform2D::Save(JSON_value * value) const
{
	Component::Save(value);
	value->AddFloat2("Position", position);
	value->AddFloat2("Size", size);
	value->AddInt("Anchor", currentAnchor);
}

void ComponentTransform2D::Load(JSON_value* value)
{
	Component::Load(value);
	position = value->GetFloat2("Position");
	size = value->GetFloat2("Size");
	currentAnchor = value->GetInt("Anchor");
}

math::float2 ComponentTransform2D::getPosition() const
{
#ifndef  GAME_BUILD
	float width = (float)App->renderer->viewGame->current_width;
	float height = (float)App->renderer->viewGame->current_height;
#else
	float width = (float)App->window->width;
	float height = (float)App->window->height;
#endif
	float horizontalCalculation = alignments[currentAnchor].x * width * 0.5;// -alignments[currentAnchor].x*(size.x / 2);
	float verticalCalculation = alignments[currentAnchor].y * height * 0.5;// -alignments[currentAnchor].y*(size.y / 2);

	return math::float2(horizontalCalculation + position.x, verticalCalculation + position.y);
}

void ComponentTransform2D::setPosition(const math::float2& position)
{
	this->position = position;
}

math::float2 ComponentTransform2D::getSize() const
{
	return size;
}
