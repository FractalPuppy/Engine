#include "ComponentText.h"
#include "GameObject.h"
#include "JSON.h"
#include "imgui.h"
#include "Application.h"
#include "ModuleFontLoader.h"
#include "ModuleUI.h"
#include <vector>
#define MAX_TEXT_LENGTH 300
#define MAX_FONT_SIZE 64
#define MAX_WRAP_WIDTH 10000
#define MAX_INTERLINE_DISTANCE 10000

Text::Text() : Component(nullptr, ComponentType::Text)
{
	font = App->fontLoader->defaultFont;
}

Text::Text(GameObject* gameobject) : Component(gameobject, ComponentType::Text)
{
	font = App->fontLoader->defaultFont;
}

Text::Text(const Text &copy) : Component(copy)
{
	font = copy.font;
	fontSize = copy.fontSize;
	text = copy.text;
	color = copy.color;
	colorHovered = copy.colorHovered;
	uiOrder = copy.uiOrder;
	offset = copy.offset;
	scaleOffset = copy.scaleOffset;
	isTextWrapped = copy.isTextWrapped;
	wrapWidth = copy.wrapWidth;
	interlineDistance = copy.interlineDistance;
	uiOrder = copy.uiOrder;
}

Text::~Text()
{
}

Component * Text::Clone() const
{
	return new Text(*this);
}

void Text::DrawProperties()
{
	if (ImGui::CollapsingHeader("Text", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool removed = Component::DrawComponentState();
		if (removed)
		{
			return;
		}
		ImGui::InputInt("UI Order##Text", &uiOrder);

		//text value
		char* imguiText = new char[MAX_TEXT_LENGTH];
		strcpy(imguiText, text.c_str());
		ImGui::InputText("##", imguiText, MAX_TEXT_LENGTH);
		text = imguiText;
		delete[] imguiText;

		//font selector
		if (ImGui::BeginCombo("Font", font.c_str()))
		{
			for (std::map<std::string, std::vector<ModuleFontLoader::Character>>::iterator it = App->fontLoader->fonts.begin(); 
				it != App->fontLoader->fonts.end(); ++it)
			{
				bool is_selected = (*it).first == font;
				if (ImGui::Selectable((*it).first.c_str(), is_selected))
				{
					font = (*it).first;
				}
				if (is_selected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		//size
		ImGui::DragFloat("Font size", &fontSize, 1.0f, 1.0f, MAX_FONT_SIZE);

		//color
		ImGui::ColorEdit4("Font color", (float*)&color);
		ImGui::ColorEdit4("Font color highlited", (float*)&colorHovered);
		ImGui::DragFloat2("Text position offset", &offset[0]);
		ImGui::DragFloat2("Text scale offset", &scaleOffset[0]);
		ImGui::Checkbox("Wrap text", &isTextWrapped);
		if (isTextWrapped)
		{
			ImGui::DragFloat("Wrap width", &wrapWidth, 1.0f, 1.0f, MAX_WRAP_WIDTH);
			ImGui::DragFloat("Interline Distance", &interlineDistance, 1.0f, 1.0f, MAX_INTERLINE_DISTANCE);
		}
		ImGui::Separator();
	}
}

void Text::Save(JSON_value *value)const
{
	Component::Save(value);
	value->AddFloat("FontSize", fontSize);
	value->AddString("text", text.c_str());
	value->AddString("font", font.c_str());
	value->AddFloat4("color", color);
	value->AddFloat4("colorHovered", colorHovered);
	value->AddFloat2("offset", offset);
	value->AddFloat2("scaleOffset", scaleOffset);
	value->AddInt("IsTextWrapped", isTextWrapped);
	value->AddFloat("WrapWidth", wrapWidth);
	value->AddFloat("InterlineDistance", interlineDistance);
	value->AddInt("UIOrder", uiOrder);
}

void Text::Load(JSON_value* value)
{
	Component::Load(value);
	fontSize = value->GetFloat("FontSize");
	text = value->GetString("text");
	font = value->GetString("font");
	color = value->GetFloat4("color");
	colorHovered = value->GetFloat4("colorHovered");
	offset = value->GetFloat2("offset");
	scaleOffset = value->GetFloat2("scaleOffset");
	isTextWrapped = value->GetInt("IsTextWrapped");
	wrapWidth = value->GetFloat("WrapWidth");
	interlineDistance = value->GetFloat("InterlineDistance");
	uiOrder = value->GetInt("UIOrder", 0);
}