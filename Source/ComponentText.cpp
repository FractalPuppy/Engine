#include "ComponentText.h"
#include "GameObject.h"
#include "JSON.h"
#include "imgui.h"
#include "Application.h"
#include "ModuleFontLoader.h"
#include "ModuleUI.h"
#include <vector>
#define MAX_TEXT_LENGTH 64
#define MAX_FONT_SIZE 64

ComponentText::ComponentText() : Component(nullptr, ComponentType::Text)
{
	font = App->fontLoader->defaultFont;
	App->ui->texts.push_back(this);
}

ComponentText::ComponentText(GameObject* gameobject) : Component(gameobject, ComponentType::Text)
{
	font = App->fontLoader->defaultFont;
	App->ui->texts.push_back(this);
}

ComponentText::ComponentText(const ComponentText &copy) : Component(copy)
{
	//copy all the shiet madafaka
	font = copy.font;
	fontSize = copy.fontSize;
	text = copy.text;
	color = copy.color;
	App->ui->texts.push_back(this);
}

ComponentText::~ComponentText()
{
	App->ui->texts.remove(this);
}

Component * ComponentText::Clone() const
{
	return new ComponentText(*this);
}

void ComponentText::DrawProperties()
{
	if (ImGui::CollapsingHeader("Text", ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool removed = Component::DrawComponentState();
		if (removed)
		{
			return;
		}
		
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
		ImGui::DragFloat2("Text offset", &offset[0]);
		ImGui::Separator();
	}
}

void ComponentText::Save(JSON_value *value)const
{
	Component::Save(value);
	value->AddFloat("FontSize", fontSize);
	value->AddString("text", text.c_str());
	value->AddString("font", font.c_str());
	value->AddFloat4("color", color);
	value->AddFloat2("offset", offset);
}

void ComponentText::Load(JSON_value* value)
{
	Component::Load(value);
	fontSize = value->GetFloat("FontSize");
	text = value->GetString("text");
	font = value->GetString("font");
	color = value->GetFloat4("color");
	offset = value->GetFloat2("offset");
}