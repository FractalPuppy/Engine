#include "Application.h"

#include "ModuleInput.h"
#include "ModuleRender.h"
#include "ModuleUI.h"

#include "ComponentButton.h"
#include "ComponentImage.h"
#include "ComponentText.h"
#include "ComponentTransform2D.h"

#include "GameObject.h"
#include "Viewport.h"
#include "imgui.h"
#include "MathGeoLib/include/Math/float2.h"
#include "JSON.h"


ComponentButton::ComponentButton() : Component(nullptr, ComponentType::Button)
{
	text = new ComponentText();
	buttonImage = new ComponentImage();
	highlightedImage = new ComponentImage();
	pressedImage = new ComponentImage();
	AssemblyButton();
}

ComponentButton::ComponentButton(GameObject* gameobject) : Component(gameobject, ComponentType::Button)
{
	text = new ComponentText();
	buttonImage = new ComponentImage();
	highlightedImage = new ComponentImage();
	pressedImage = new ComponentImage();
	AssemblyButton();
}

ComponentButton::ComponentButton(const ComponentButton& copy) : Component(copy)
{
	text = (ComponentText*) copy.text->Clone();
	buttonImage = (ComponentImage*)copy.buttonImage->Clone();
	highlightedImage = (ComponentImage*)copy.highlightedImage->Clone();
	pressedImage = (ComponentImage*)copy.pressedImage->Clone();
	rectTransform = (ComponentTransform2D*)copy.rectTransform->Clone();
}


ComponentButton::~ComponentButton()
{
	RELEASE(text);
	RELEASE(buttonImage);
	RELEASE(highlightedImage);
	RELEASE(pressedImage);
}

Component* ComponentButton::Clone() const
{
	return new ComponentButton(*this);;
}

void ComponentButton::DrawProperties()
{
	ImGui::PushID(0);
	text->DrawProperties();
	ImGui::Text("Button image");
	ImGui::PushID(1);
	buttonImage->DrawProperties();	
	ImGui::Text("Highlighted image");
	ImGui::PushID(2);
	highlightedImage->DrawProperties();
	ImGui::Text("Pressed image");
	ImGui::PushID(3);
	pressedImage->DrawProperties();
	ImGui::PopID();
	ImGui::PopID();
	ImGui::PopID();
	ImGui::PopID();
}

void ComponentButton::Save(JSON_value* value) const
{
	Component::Save(value);
	JSON_value* textValue = value->CreateValue(rapidjson::kObjectType);	
	text->Save(textValue);
	JSON_value* bValue = value->CreateValue(rapidjson::kObjectType);
	buttonImage->Save(bValue);
	JSON_value* hValue = value->CreateValue(rapidjson::kObjectType);
	highlightedImage->Save(hValue);
	JSON_value* pValue = value->CreateValue(rapidjson::kObjectType);
	pressedImage->Save(pValue);
	value->AddValue("text", *textValue);
	value->AddValue("buttonImage", *bValue);
	value->AddValue("highlightedImage", *hValue);
	value->AddValue("pressedImage", *pValue);
}

void ComponentButton::Load(JSON_value* value)
{
	RELEASE(text);
	RELEASE(buttonImage);
	RELEASE(highlightedImage);
	RELEASE(pressedImage);	
	Component::Load(value);
	text = new ComponentText();
	text->Load(value->GetValue("text"));
	buttonImage = new ComponentImage();
	buttonImage->Load(value->GetValue("buttonImage"));
	highlightedImage = new ComponentImage();
	highlightedImage->Load(value->GetValue("highlightedImage"));
	pressedImage = new ComponentImage();
	pressedImage->Load(value->GetValue("pressedImage"));	
	AssemblyButton();
}

void ComponentButton::Update()
{
	math::float2 mouse = reinterpret_cast<const float2&>(App->input->GetMousePosition());	
	float screenX = mouse.x - App->renderer->viewGame->winPos.x - (App->ui->currentWidth * .5f);
	float screenY = mouse.y - App->renderer->viewGame->winPos.y - (App->ui->currentHeight * .5f);
	math::float2 pos = rectTransform->getPosition();
	math::float2 size = rectTransform->getSize();
	float buttonX = pos.x;
	float buttonY = pos.y;
	
	math::float2 buttonMin = float2(buttonX - size.x *.5f, -buttonY - size.y *.5f);
	math::float2 buttonMax = float2(buttonX + size.x *.5f, -buttonY + size.y *.5f);
	if (screenX > buttonMin.x && screenX < buttonMax.x && screenY > buttonMin.y && screenY < buttonMax.y)
	{
		isHovered = true;
		buttonImage->enabled = false;
		highlightedImage->enabled = true;
		pressedImage->enabled = false;
		text->isHovered = true;
	}
	else
	{
		isHovered = false;
		buttonImage->enabled = true && !isSelected;
		highlightedImage->enabled = false || isSelected;
		pressedImage->enabled = false;
		text->isHovered = false;
	}

	if (isHovered && App->input->GetMouseButtonDown(1) == KEY_DOWN)
	{
		isPressed = true;
		buttonImage->enabled = false;
		highlightedImage->enabled = false;
		pressedImage->enabled = true;
	}
	else
	{
		isPressed = false;
		pressedImage->enabled = false;
	}
}

void ComponentButton::Enable(bool enable)
{
	Component::Enable(enable);
	isPressed = false;
	isHovered = false;
	isSelected = false;
	highlightedImage->enabled = false;
	pressedImage->enabled = false;
	buttonImage->enabled = true;
}

void ComponentButton::AssemblyButton() 
{
	text->gameobject = gameobject;
	buttonImage->gameobject = gameobject;
	highlightedImage->gameobject = gameobject;
	pressedImage->gameobject = gameobject;
	rectTransform = (ComponentTransform2D*)gameobject->GetComponent(ComponentType::Transform2D);
	buttonImage->enabled = true;
	highlightedImage->enabled = false;
	pressedImage->enabled = false;
}