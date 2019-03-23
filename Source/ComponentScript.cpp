#include "Application.h"

#include "ModuleScript.h"
#include "ComponentScript.h"

#include "GameObject.h"
#include "BaseScript.h"
#include "JSON.h"
#include "imgui.h"

#include <map>

ComponentScript::ComponentScript(GameObject* gameobject) : Component(gameobject, ComponentType::Script)
{
}

ComponentScript::ComponentScript(const ComponentScript& component) : Component(component)
{
	scriptName = component.scriptName;
	if (component.script != nullptr)
	{
		SetScript(scriptName);
	}
}

ComponentScript::~ComponentScript()
{
	if (script != nullptr)
	{
		App->scripting->RemoveScript(scriptName, script);
	}
}

ComponentScript * ComponentScript::Clone() const
{
	return new ComponentScript(*this);
}

void ComponentScript::DrawProperties()
{
	ImGui::PushID(this);
	if (ImGui::CollapsingHeader(scriptName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool removed = Component::DrawComponentState();
		if (removed)
		{
			return;
		}

		ImGui::Text("Select Script");
		if (ImGui::BeginCombo("", scriptName.c_str()))
		{
			std::map<std::string, int>::const_iterator it;
			for (it = App->scripting->scripts.begin(); it != App->scripting->scripts.end(); it++)
			{
				bool isSelected = (scriptName == it->first);
				if (ImGui::Selectable(it->first.c_str(), isSelected) && scriptName != it->first)
				{
					SetScript(it->first);
				}
				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		ImGuiContext * context = ImGui::GetCurrentContext();
		if (script != nullptr)
		{
			script->Expose(context);
		}
	}
	ImGui::PopID();
}

void ComponentScript::Save(JSON_value* value) const
{
	Component::Save(value);
	if (script != nullptr)
	{
		value->AddString("script", scriptName.c_str());
		JSON_value *scriptInfo = value->CreateValue();
		script->Serialize(scriptInfo);
		value->AddValue("scriptInfo", *scriptInfo);;
	}
}

void ComponentScript::Load(JSON_value* value)
{
	Component::Load(value);
	const char* retrievedName = value->GetString("script");
	if (retrievedName != nullptr)
	{
		SetScript(retrievedName);
		if (script != nullptr)
		{
			JSON_value* scriptInfo = value->GetValue("scriptInfo");
			if (scriptInfo != nullptr)
			{
				script->DeSerialize(scriptInfo);
			}
		}
	}
}

void ComponentScript::SetScript(const std::string& name)
{
	if (script != nullptr)
	{
		App->scripting->RemoveScript(scriptName, script);
	}
	script = App->scripting->AddScript(name);
	if (script != nullptr)
	{
		script->SetApp(App);
		script->SetGameObject(gameobject);
		scriptName = name;
	}
	else
	{
		scriptName = "Script Not Found";
	}
}

