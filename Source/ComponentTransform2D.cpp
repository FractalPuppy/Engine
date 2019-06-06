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

Transform2D::Transform2D(GameObject* gameobject) : Component(gameobject, ComponentType::Transform2D)
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

Transform2D::Transform2D(const Transform2D & component) : Component(component)
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

Transform2D::~Transform2D()
{
}

Component * Transform2D::Clone() const
{
	return new Transform2D(*this);
}

void Transform2D::DrawProperties()
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
			int newAnchor = -1;

			//top
			if (currentAnchor == TOPLEFT) ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.f, 0.6f, 0.6f));
			if (ImGui::Button("Top Left")) newAnchor = TOPLEFT;
			if (currentAnchor == TOPLEFT) ImGui::PopStyleColor();

			ImGui::SameLine();
			if (currentAnchor == TOPCENTER) ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.f, 0.6f, 0.6f));
			if (ImGui::Button("Top Center")) newAnchor = TOPCENTER;
			if (currentAnchor == TOPCENTER) ImGui::PopStyleColor();

			ImGui::SameLine();
			if (currentAnchor == TOPRIGHT) ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.f, 0.6f, 0.6f));
			if (ImGui::Button("Top Right")) newAnchor = TOPRIGHT;
			if (currentAnchor == TOPRIGHT) ImGui::PopStyleColor();

			//middle
			if (currentAnchor == MIDDLELEFT) ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.f, 0.6f, 0.6f));
			if (ImGui::Button("Middle Left")) newAnchor = MIDDLELEFT;
			if (currentAnchor == MIDDLELEFT) ImGui::PopStyleColor();

			ImGui::SameLine();
			if (currentAnchor == MIDDLECENTER) ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.f, 0.6f, 0.6f));
			if (ImGui::Button("Middle Center"))	newAnchor = MIDDLECENTER;
			if (currentAnchor == MIDDLECENTER) ImGui::PopStyleColor();

			ImGui::SameLine();
			if (currentAnchor == MIDDLERIGHT) ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.f, 0.6f, 0.6f));
			if (ImGui::Button("Middle Right")) newAnchor = MIDDLERIGHT;
			if (currentAnchor == MIDDLERIGHT) ImGui::PopStyleColor();

			//bottom
			if (currentAnchor == BOTTOMLEFT) ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.f, 0.6f, 0.6f));
			if (ImGui::Button("Bottom Left")) newAnchor = BOTTOMLEFT;
			if (currentAnchor == BOTTOMLEFT) ImGui::PopStyleColor();

			ImGui::SameLine();
			if (currentAnchor == BOTTOMCENTER) ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.f, 0.6f, 0.6f));
			if (ImGui::Button("Bottom Center"))	newAnchor = BOTTOMCENTER;
			if (currentAnchor == BOTTOMCENTER) ImGui::PopStyleColor();

			ImGui::SameLine();
			if (currentAnchor == BOTTOMRIGHT) ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.f, 0.6f, 0.6f));
			if (ImGui::Button("Bottom Right")) newAnchor = BOTTOMRIGHT;
			if (currentAnchor == BOTTOMRIGHT) ImGui::PopStyleColor();

			if (newAnchor > -1)
				currentAnchor = newAnchor;
		}
	}
}

bool Transform2D::CleanUp()
{
	alignments.clear();//clears array
	alignments.shrink_to_fit();//reduces the capacity to fit its size, which is 0
	return true;
}

void Transform2D::Save(JSON_value * value) const
{
	Component::Save(value);
	value->AddFloat2("Position", position);
	value->AddFloat2("Size", size);
	value->AddInt("Anchor", currentAnchor);
}

void Transform2D::Load(JSON_value* value)
{
	Component::Load(value);
	position = value->GetFloat2("Position");
	size = value->GetFloat2("Size");
	currentAnchor = value->GetInt("Anchor");
}

math::float2 Transform2D::getPosition() const
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

void Transform2D::setPosition(const math::float2& position)
{
	this->position = position;
}

void Transform2D::SetPositionUsingAligment(math::float2& newPosition)
{

#ifndef  GAME_BUILD
	float width = (float)App->renderer->viewGame->current_width;
	float height = (float)App->renderer->viewGame->current_height;
#else
	float width = (float)App->window->width;
	float height = (float)App->window->height;
#endif
	float horizontalCalculation = alignments[currentAnchor].x * width * 0.5;
	float verticalCalculation = alignments[currentAnchor].y * height * 0.5;

	this->position = math::float2(newPosition.x - horizontalCalculation, newPosition.y - verticalCalculation);
}

math::float2 Transform2D::getSize() const
{
	return size;
}

void Transform2D::SetSize(math::float2 newSize)
{
	size = newSize;
}

